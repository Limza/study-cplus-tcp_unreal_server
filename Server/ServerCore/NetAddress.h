#pragma once

/* ---------------------------------
 *	NetAddress
 --------------------------------- */
class NetAddress
{
public:
	NetAddress() = default;
	explicit NetAddress(SOCKADDR_IN sockAddr);
	NetAddress(const std::wstring& ip, uint16 port);

	SOCKADDR_IN& GetSockAddr() { return _sockAddr; }
	std::wstring GetIpAddress() const;
	uint16 GetPort() const { return ::ntohs(_sockAddr.sin_port); }

public:
	static IN_ADDR Ip2Address(const WCHAR* ip);

private:
	SOCKADDR_IN _sockAddr = {};
};

