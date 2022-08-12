#pragma once

#include <string>
#include "irc/Server.hpp"
#include <poll.h>

namespace ft
{

class Client
{
private:
	int _socket;
	std::string _username, _nickname, _realname;
	std::string _address;
	unsigned short _port;
	Server& _server;

	bool	_has_entered;
	std::string _message_buffer;

public:
	Client(Server& server, int socket, const std::string& address, unsigned short port);

	int	socket() const;
	const std::string& address() const;
	unsigned short port() const;
	void append_to_buffer(const std::string& str);
	void treat_command();

private:
};

}