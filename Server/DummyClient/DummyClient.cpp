#include "pch.h"
#include "Session.h"
#include "Service.h"
#include "ThreadManager.h"
#include "ServerPacketHandler.h"

using namespace std;

class ServerSession final : public PacketSession
{
public:
	~ServerSession() override
	{
		cout << "ServerSession Destructor\n";
	}

protected:
	void OnConnected() override
	{
		// cout << "Connected To Server" << '\n';

		const Protocol::C_LOGIN pkt;
		auto sendBufferRef = ServerPacketHandler::MakeSendBuffer(pkt);
		Send(sendBufferRef);
	}

	void OnRecvPacket(BYTE* buffer, const int32 len) override
	{
		auto session = GetPacketSessionRef();

		ServerPacketHandler::HandlePacket(session, buffer, len);
	}

	void OnSend(const int32 len) override
	{
		// cout << "OnSend Len = " << len << '\n';
	}

	void OnDisconnected() override
	{
		// cout << "Disconnected\n";
	}
};

int main()
{
	ServerPacketHandler::Init();

	this_thread::sleep_for(1s);

	const ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777)
		, MakeShared<IocpCore>()
		, MakeShared<ServerSession>,
		1);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 2; ++i)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	Protocol::C_CHAT chatPkt;
	chatPkt.set_msg(reinterpret_cast<const char*>(u8"Hello world"));
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(chatPkt);

	while (true)
	{
		service->Broadcast(sendBuffer);
		this_thread::sleep_for(1s);
	}

	GThreadManager->Join();
}
