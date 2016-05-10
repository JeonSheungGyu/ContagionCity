#pragma once

// DB_Server -> Login_Server
class PacketDispatcher_DB_Server
{
public:
	static void PermisionLogin(char*buf, const unsigned short id);
};


// Client -> Login_Server
class PacketDispatcher_Client
{
public:
	static void RequestLogin(char*buf, const unsigned short id);
};

