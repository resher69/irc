#include <iostream>
#include "irc/Client.hpp"
#include "Colors.hpp"

namespace ft
{

Client::Client(Server& server, int socket, const std::string& address, unsigned short port)
	:	_socket(socket), _username(), _nickname(), _realname(),
		_address(address), _port(port), _server(server), _has_entered(false),
		_message_buffer()
{
	if (!this->_server.has_password()) {
		_has_entered = true;
	}

	std::cout << INFO << "New connection from " << this->_address << ":" << this->_port << std::endl;
}

int	Client::socket() const
{
	return this->_socket;
}

const std::string& Client::address() const
{
	return this->_address;
}

unsigned short Client::port() const
{
	return this->_port;
}

void Client::append_to_buffer(const std::string& str)
{
	this->_message_buffer += str;
}

void Client::treat_command()
{
	// Removing the '\n'
	this->_message_buffer.erase(this->_message_buffer.size() - 1, 1);

	// The RFC states that every command is delimited by a CRLF (\r\n).
	// To support the RFC, we check if there is a carriage return before the newline, and we remove it
	if (	!this->_message_buffer.empty()
			&& this->_message_buffer[this->_message_buffer.size() - 1] == '\r'	)
	{
		this->_message_buffer.erase(this->_message_buffer.size() - 1, 1);
	}

	std::cout << CLIENT << this->address() << ":" << this->port() << " :" << this->_message_buffer << std::endl;
	// TODO: treat command...

	this->_message_buffer.clear();
}

}