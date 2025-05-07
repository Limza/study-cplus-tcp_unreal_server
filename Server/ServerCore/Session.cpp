#include "pch.h"
#include "Session.h"

#include "Service.h"
#include "SocketUtils.h"

/* ------------------------------
 *	Session
 ------------------------------*/
Session::Session() : _recvBuffer(_bufferSize)
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(SendBufferRef sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool registerSend = false;

	{
		WRITE_LOCK;

		_sendQueue.push(sendBuffer);

		if (_sendRegistered.exchange(true) == false)
			registerSend = true;
	}

	if (registerSend)
		RegisterSend();
}

bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	// TEMP
	std::wcout << L"Disconnect : " << cause << '\n';

	RegisterDisConnect();
}

void Session::SetNetAddress(const NetAddress address)
{ _netAddress = address; }

NetAddress Session::GetAddress() const
{ return _netAddress; }

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket); // NOLINT(performance-no-int-to-ptr)
}

void Session::Dispatch(class IocpEvent* iocpEvent, const int32 numOfBytes)
{
	switch (iocpEvent->GetEventType())
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Disconnect:
		ProcessDisConnect();
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case EventType::Send:
		ProcessSend(numOfBytes);
		break;
	case EventType::Accept:
	case EventType::PreRecv:
		break;
	}
}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (GetService()->GetType() != ServiceType::Client)
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_socket,  /*남는 포트 아무거나*/0) == false)
		return false;

	_connectEvent.Init();
	_connectEvent.SetOwner(shared_from_this()); // ADD_REF

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetAddress().GetSockAddr();
	const auto connectResult = SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr)
		, sizeof(sockAddr), nullptr, 0
		, &numOfBytes, &_connectEvent);

	if (connectResult == false)
	{
		if (::WSAGetLastError() != WSA_IO_PENDING)
		{
			_connectEvent.SetOwner(nullptr); // RELEASE_REF
			return false;
		}
	}

	return true;
}

bool Session::RegisterDisConnect()
{
	_disConnectEvent.Init();
	_disConnectEvent.SetOwner(shared_from_this()); // ADD_REF

	const auto disconnectResult = SocketUtils::DisconnectEx(_socket
		, &_disConnectEvent, TF_REUSE_SOCKET, 0);
	if (false == disconnectResult)
	{
		if (const auto errorCode = ::WSAGetLastError(); errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_disConnectEvent.SetOwner(nullptr); // RELEASE_REF
			return false;
		}
	}

	return true;
}

void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init();
	_recvEvent.SetOwner(shared_from_this()); // ADD_REF

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	const auto recvResult = ::WSARecv(_socket, &wsaBuf, 1
		, OUT & numOfBytes, OUT & flags, &_recvEvent, nullptr);
	if (recvResult == SOCKET_ERROR)
	{
		if (const auto errorCode = ::WSAGetLastError(); errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_recvEvent.SetOwner(nullptr); // RELEASE_REF
		}
	}
}

void Session::RegisterSend()
{
	if (IsConnected() == false)
		return;

	_sendEvent.Init();
	_sendEvent.SetOwner(shared_from_this()); // ADD_REF

	// 보낼 데이터를 sendEvent에 등록
	{
		WRITE_LOCK;

		uint32 writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			SendBufferRef sendBuffer = _sendQueue.front();

			writeSize += sendBuffer->WriteSize();

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	// Scatter-Gather
	Vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());
	for (const auto& sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<ULONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	const auto sendResult = ::WSASend(_socket, wsaBufs.data()
		, static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes
		, 0, &_sendEvent, nullptr);

	if (sendResult == SOCKET_ERROR)
	{
		const auto errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_sendEvent.SetOwner(nullptr); // RELEASE_REF
			_sendEvent.sendBuffers.clear(); // RELEASE_REF
			_sendRegistered.store(false);
		}
	}
}

void Session::ProcessConnect()
{
	_connectEvent.SetOwner(nullptr); // RELEASE_REF
	_connected.store(true);

	GetService()->AddSession(GetSessionRef());
	OnConnected();
	RegisterRecv();
}

void Session::ProcessDisConnect()
{
	_disConnectEvent.SetOwner(nullptr); // RELEASE_REF

	OnDisconnected();
	GetService()->ReleaseSession(GetSessionRef());
}

void Session::ProcessRecv(const int32 numOfBytes)
{
	_recvEvent.SetOwner(nullptr); // RELEASE_REF

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	const int32 dataSize = _recvBuffer.DataSize();
	const int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	_recvBuffer.Clean();

	RegisterRecv();
}

void Session::ProcessSend(const int32 numOfBytes)
{
	_sendEvent.SetOwner(nullptr); // RELEASE_REF
	_sendEvent.sendBuffers.clear(); // RELEASE_REF

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	OnSend(numOfBytes);

	WRITE_LOCK;
	if (_sendQueue.empty())
		_sendRegistered.store(false);
	else
		RegisterSend();
}

void Session::HandleError(const int32 errorCode)
{
	switch (errorCode)
	{
		case WSAECONNRESET:
		case WSAECONNABORTED:
			Disconnect(L"HandleError");
			break;
		default:
			std::cout << "Handle Error : " << errorCode << '\n';
			break;
	}
}


/* ------------------------------
 *	PacketSession
 ------------------------------*/

PacketSession::PacketSession()
= default;

PacketSession::~PacketSession()
= default;

// [size][id][data..][size]...
int32 PacketSession::OnRecv(BYTE* buffer, const int32 len)
{
	int32 processLen = 0;

	while (true)
	{
		const int32 dataSize = len - processLen;
		// 최소한 헤더는 파싱할 수 있어야 한다
		if (dataSize < sizeof(PacketHeader))  // NOLINT(clang-diagnostic-sign-compare)
			break;

		const auto [size, _] = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));
		if (dataSize < size)  // NOLINT(modernize-use-integer-sign-comparison)
			break;

		// 패킷 조립 성공
		OnRecvPacket(&buffer[0], size);

		processLen += size;
	}

	return processLen;
}
