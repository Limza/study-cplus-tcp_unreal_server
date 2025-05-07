#include "pch.h"

#include "Service.h"

#include <utility>
#include "Session.h"
#include "Listener.h"

/* ------------------------------
 * Service
 ------------------------------ */
Service::Service(const ServiceType type, const NetAddress address
	, IocpCoreRef core, SessionFactory factory, const int32 maxSessionCount)
	: _type(type), _netAddress(address), _iocpCore(std::move(core))
	  , _sessionFactory(std::move(factory)), _maxSessionCount(maxSessionCount)
{
}

Service::~Service()
{
}

void Service::CloseService()
{
}

void Service::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (const auto& session : _sessions)
	{
		if (session->IsConnected() == false)
			continue;
		session->Send(sendBuffer);
	}
}

SessionRef Service::CreateSession()
{
	SessionRef session = _sessionFactory();
	session->SetService(shared_from_this());

	if (_iocpCore->Register(session) == false)
		return nullptr;

	return session;
}

void Service::AddSession(SessionRef&& session)
{
	WRITE_LOCK;
	_sessionCount++;
	_sessions.insert(session);
}

void Service::ReleaseSession(SessionRef&& session)
{
	WRITE_LOCK;
	ASSERT_CRASH(_sessions.erase(session) != 0);
	_sessionCount--;
}


/* ------------------------------
 * ClientService
 ------------------------------ */
ClientService::ClientService(const NetAddress targetAddress, IocpCoreRef core
	, const SessionFactory& factory, const int maxSessionCount)
	: Service(ServiceType::Client, targetAddress, std::move(core), factory, maxSessionCount)
{
}

bool ClientService::Start()
{
	if (CanStart() == false)
		return false;

	const int32 sessionCount = GetMaxSessionCount();
	for (int32 i = 0; i < sessionCount; ++i)
	{
		auto session = CreateSession();
		if (session->Connect() == false)
			return false;
	}

	return true;
}


/* ------------------------------
 * ServerService
 ------------------------------ */
ServerService::ServerService(const NetAddress address, IocpCoreRef core, 
	const SessionFactory& factory, const int maxSessionCount)
	: Service(ServiceType::Server, address, std::move(core), factory, maxSessionCount)
{
}

bool ServerService::Start()
{
	if (CanStart() == false)
		return false;

	_listener = MakeShared<Listener>();
	if (_listener == nullptr)
		return false;

	const auto service = 
		std::static_pointer_cast<ServerService>(shared_from_this());
	if (_listener->StartAccept(service) == false)
		return false;

	return true;
}

void ServerService::CloseService()
{
	Service::CloseService();
}
