#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"

class Service;

/* ------------------------------
 *	Session
 ------------------------------*/
class Session : public IocpObject
{
	// core 안에서만 접근할 수 있도록
	friend class Listener;
	friend class IocpCore;
	friend class Service;

	static constexpr int32 _bufferSize = 0x10000; // 64KB

public:
	Session();
	~Session() override;
	NON_COPYABLE_CLASS(Session);

public:
	void				Send(SendBufferRef sendBuffer);
	bool				Connect();
	void				Disconnect(const WCHAR* cause);
	void				SetService(std::shared_ptr<Service> service) { _service = service; }
	std::shared_ptr<Service> GetService() const { return _service.lock(); }

public:
	void		SetNetAddress(const NetAddress address);
	NetAddress	GetAddress() const;
	SOCKET		GetSocket() const { return _socket; }
	bool		IsConnected() const { return _connected; }
	SessionRef	GetSessionRef() { return std::static_pointer_cast<Session>(shared_from_this()); }

protected:
	virtual void	OnConnected() {}
	virtual int32	OnRecv(BYTE* buffer, const int32 len) { return len; }
	virtual void	OnSend(int32 len) {}
	virtual void	OnDisconnected() {}

private:
	HANDLE	GetHandle() override;
	void	Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	bool	RegisterConnect();
	bool	RegisterDisConnect();
	void	RegisterRecv();
	void	RegisterSend();

	void	ProcessConnect();
	void	ProcessDisConnect();
	void	ProcessRecv(int32 numOfBytes);
	void	ProcessSend(int32 numOfBytes);

	void	HandleError(int32 errorCode);
	
private:
	std::weak_ptr<Service>	_service;
	SOCKET				_socket = INVALID_SOCKET;
	NetAddress			_netAddress = {};
	Atomic<bool>		_connected = false;

private:
	USE_LOCK;

	RecvBuffer				_recvBuffer;

	Queue<SendBufferRef>	_sendQueue;
	Atomic<bool>			_sendRegistered = false;

private:
	ConnectEvent		_connectEvent;
	DisConnectEvent		_disConnectEvent;
	RecvEvent			_recvEvent;
	SendEvent			_sendEvent;
};


/* ------------------------------
 *	PacketSession
 ------------------------------*/

struct PacketHeader
{
	uint16 size = 0;
	uint16 id = 0; // 프로토콜 ID
};

class PacketSession : public Session
{
public:
	PacketSession();
	~PacketSession() override;
	NON_COPYABLE_CLASS(PacketSession);

	PacketSessionRef GetPacketSessionRef()
	{
		return std::static_pointer_cast<PacketSession>(shared_from_this());
	}

protected:
	int32 OnRecv(BYTE* buffer, const int32 len) final;
	virtual void OnRecvPacket(BYTE* buffer, const int32 len) = 0;
};