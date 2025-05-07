#pragma once
#include "IocpCore.h"

class AcceptEvent;

/* ------------------------------
 *	Listener
 ------------------------------*/
class Listener final : public IocpObject
{
public:
	Listener() = default;
	~Listener() override;
	NON_COPYABLE_CLASS(Listener);

public:
	bool StartAccept(ServerServiceRef service);
	void CloseSocket();

public:
	HANDLE GetHandle() override;
	void Dispatch(IocpEvent* iocpEvent, int32 numOfBytes) override;

private:
	void RegisterAccept(AcceptEvent* acceptEvent) const;
	void ProcessAccept(AcceptEvent* acceptEvent) const;

private:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;
	ServerServiceRef _service;
};

