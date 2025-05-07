#pragma once

class Session;
enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	PreRecv,
	Recv,
	Send,
};

/* ------------------------------
 * IocpEvent
 ------------------------------ */
class IocpEvent : public OVERLAPPED
{
public:
	// OVERLAPPED 를 사용해야 하기 때문에, virtual 함수를 사용할 안함.
	// 메모리의 가장 처음에는 OVERLAPPED 가 있어야 한다.
	explicit  IocpEvent(EventType type);

public:
	void Init();

	[[nodiscard]] EventType GetEventType() const { return _eventType; }
	[[nodiscard]] const IocpObjectRef& GetOwner() const { return _owner; }
	void SetOwner(IocpObjectRef&& owner) { _owner = std::move(owner); }

private:
	EventType _eventType;
	IocpObjectRef _owner;
};

/* ------------------------------
 * ConnectEvent
 ------------------------------ */
class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) {}
};

/* ------------------------------
 * DisConnectEvent
 ------------------------------ */
class DisConnectEvent : public IocpEvent
{
public:
	DisConnectEvent() : IocpEvent(EventType::Disconnect) {}
};

/* ------------------------------
 * AcceptEvent
 ------------------------------ */
class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) {}

public:
	[[nodiscard]] const SessionRef& GetSession() const { return _session; }
	void SetSession(SessionRef session) { _session = session; }

private:
	SessionRef _session = nullptr;
};

/* ------------------------------
 * RecvEvent
 ------------------------------ */
class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}
};

/* ------------------------------
 * SendEvent
 ------------------------------ */
class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) {}

	Vector<SendBufferRef> sendBuffers;
};