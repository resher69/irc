#pragma once

#include <string>
#include "irc/Server.hpp"
#include <poll.h>
#include <queue>

namespace ft
{

class Client
{
private:
	int _socket;
	std::string _username, _nickname;
	std::string _address;
	unsigned short _port;
	Server& _server;

	bool	_has_logged;
	std::string _message_buffer;

	std::queue<std::string>	_response_queue;

public:
	Client(Server& server, int socket, const std::string& address, unsigned short port);

	int	socket() const;
	const std::string& address() const;
	unsigned short port() const;
	void append_to_buffer(const std::string& str);
	void treat_command();
	void disconnect();

	void send(const std::string& data);
	size_t	response_queue_size() const;
	std::string response_queue_pop();

	bool is_logged() const;
	void set_logged();

	bool is_registered() const;

	void set_nickname(const std::string& nick);
	void set_username(const std::string& user);
	const std::string& nickname() const;
	const std::string& username() const;

private:
};

}
