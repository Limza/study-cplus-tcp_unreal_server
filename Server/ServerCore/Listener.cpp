#include "pch.h"
#include "Listener.h"

#include "IocpEvent.h"
#include "Service.h"
#include "Session.h"
#include "SocketUtils.h"


/* ------------------------------
 *	Listener
 ------------------------------*/
Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* acceptEvent : _acceptEvents)
	{
		xdelete(acceptEvent);
	}
}

bool Listener::StartAccept(ServerServiceRef service)
{
	_service = service;
	if (_service == nullptr)
		return false;

	_socket = SocketUtils::CreateSocket();
	if (_socket == INVALID_SOCKET)
		return false;

	if (service->GetIocpCore()->Register(shared_from_this()) == false)
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	if (SocketUtils::Bind(_socket, service->GetAddress()) == false)
		return false;

	if (SocketUtils::Listen(_socket) == false)
		return false;

	const int32 acceptCount = _service->GetMaxSessionCount();
	for (int32 i = 0; i < acceptCount; ++i)
	{
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();
		acceptEvent->SetOwner(shared_from_this());
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return true;
}

void Listener::CloseSocket()
{
	SocketUtils::Close(_socket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket); // NOLINT(performance-no-int-to-ptr)
}

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	ASSERT_CRASH(iocpEvent->GetEventType() == EventType::Accept);
	const auto acceptEvent = static_cast<AcceptEvent*>(iocpEvent);  // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
	ProcessAccept(acceptEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent) const
{
	const SessionRef session = _service->CreateSession();

	acceptEvent->Init();
	acceptEvent->SetSession(session);

	DWORD bytesReceived = 0;
	const BOOL result = SocketUtils::AcceptEx(_socket
		, session->GetSocket(), session->_recvBuffer.WritePos(), 0
		, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16
		, OUT &bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent));

	if (result == FALSE)
	{
		const int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent) const
{
	const SessionRef session = acceptEvent->GetSession();

	if (false == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);
	const int peerNameResult = ::getpeername(session->GetSocket()
		, OUT reinterpret_cast<SOCKADDR*>(&sockAddress)
		, OUT & sizeOfSockAddr);
	if (peerNameResult == SOCKET_ERROR)
	{
		RegisterAccept(acceptEvent);
		return;
	}

	session->SetNetAddress(NetAddress(sockAddress));
	session->ProcessConnect();
	RegisterAccept(acceptEvent);
}
