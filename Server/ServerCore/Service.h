#pragma once
#include "NetAddress.h"
#include <functional>

using std::function;

enum class ServiceType : uint8
{
	Server,
	Client
};

/* ------------------------------
 * Service
 ------------------------------ */

using SessionFactory = function<SessionRef(void)>;

class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(ServiceType type, NetAddress address, IocpCoreRef core
		, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~Service();
	NON_COPYABLE_CLASS(Service);

public:
	virtual bool Start() = 0;
	virtual void CloseService();

	void		SetSessionFactory(const SessionFactory& func) { _sessionFactory = func; }
	void		Broadcast(SendBufferRef sendBuffer);
	SessionRef	CreateSession();
	void		AddSession(SessionRef&& session);
	void		ReleaseSession(SessionRef&& session);

public:
	bool	CanStart() const { return _sessionFactory != nullptr; }
	int32	GetCurrentSessionCount() const { return _sessionCount; }
	int32	GetMaxSessionCount() const { return _maxSessionCount; }
	ServiceType		GetType() const { return _type; }
	NetAddress&	GetAddress() { return _netAddress; }
	const IocpCoreRef&	 GetIocpCore() const { return _iocpCore; }

protected:
	USE_LOCK;

	ServiceType _type;
	NetAddress _netAddress = {};
	IocpCoreRef _iocpCore;

	Set<SessionRef> _sessions;
	int32 _sessionCount = 0;
	SessionFactory _sessionFactory;
	int32 _maxSessionCount = 0;
};


/* ------------------------------
 * ClientService
 ------------------------------ */
class ClientService final : public Service
{
public:
	ClientService(NetAddress targetAddress, IocpCoreRef core
		, const SessionFactory& factory, int maxSessionCount = 1);
	~ClientService() override = default;
	NON_COPYABLE_CLASS(ClientService);

	bool Start() override;
};


 /* ------------------------------
  * ServerService
  ------------------------------ */
class ServerService final : public Service
{
public:
	ServerService(NetAddress address, IocpCoreRef core
		, const SessionFactory& factory, int maxSessionCount = 1);
	~ServerService() override = default;
	NON_COPYABLE_CLASS(ServerService);

	bool Start() override;
	void CloseService() override;

private:
	ListenerRef _listener = nullptr;
};