#include "pch.h"
#include "SocketUtils.h"

/* ------------------------------
 *	SocketUtils
 ------------------------------ */

LPFN_CONNECTEX		SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX	SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX		SocketUtils::AcceptEx = nullptr;

void SocketUtils::Init()
{
	WSADATA wsaData;
	ASSERT_CRASH(::WSAStartup(MAKEWORD(2, 2), OUT & wsaData) == 0);

	// 런타임에 통신에 필요한 주소를 얻어온다
	SOCKET dummySocket = CreateSocket();
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));
	Close(dummySocket);
}

void SocketUtils::Clear()
{
	::WSACleanup();
}

bool SocketUtils::BindWindowsFunction(const SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	const int result =::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER
		, &guid, sizeof(guid), reinterpret_cast<LPVOID>(fn)
		, sizeof(*fn), OUT & bytes, nullptr, nullptr);
	return SOCKET_ERROR != result;
}

SOCKET SocketUtils::CreateSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP
		, nullptr, 0, WSA_FLAG_OVERLAPPED);
}

bool SocketUtils::SetLinger(const SOCKET socket, const uint16 onOff, const uint16 linger)
{
	LINGER option;
	option.l_onoff = onOff;
	option.l_linger = linger;
	return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, option);
}

bool SocketUtils::SetReuseAddress(const SOCKET socket, const bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketUtils::SetRecvBufferSize(const SOCKET socket, const int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBufferSize(const SOCKET socket, const int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::SetTcpNoDelay(const SOCKET socket, const bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, TCP_NODELAY, flag);
}

// Listen Socket의 특성을 Client Socket에 그대로 적용
bool SocketUtils::SetUpdateAcceptSocket(const SOCKET socket, const SOCKET listenSocket)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT
		, listenSocket);
}

bool SocketUtils::Bind(const SOCKET socket, NetAddress netAddr)
{
	return SOCKET_ERROR != ::bind(socket
		, reinterpret_cast<const SOCKADDR*>(&netAddr.GetSockAddr())
		, sizeof(SOCKADDR_IN));
}

bool SocketUtils::BindAnyAddress(const SOCKET socket, const uint16 port)
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress)
		, sizeof(myAddress));
}

bool SocketUtils::Listen(const SOCKET socket, const int32 backlog)
{
	return SOCKET_ERROR != ::listen(socket, backlog);
}

void SocketUtils::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
		::closesocket(socket);
	socket = INVALID_SOCKET;
}
