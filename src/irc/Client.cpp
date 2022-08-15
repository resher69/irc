#include <iostream>
#include "irc/Client.hpp"
#include "Colors.hpp"
#include <unistd.h>

namespace ft
{

Client::Client(Server& server, int socket, const std::string& address, unsigned short port)
	:	_socket(socket), _username(), _nickname(), _address(address),
		_port(port), _server(server), _has_logged(false), _message_buffer()
{
	if (!this->_server.has_password()) {
		_has_logged = true;
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

void Client::disconnect()
{
	::close(this->_socket);
	this->_socket = -1;
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

	if (this->_message_buffer.empty()) {
		return ;
	}

	std::cout << CLIENT << this->address() << ":" << this->port() << ": '" << this->_message_buffer << "'" << std::endl;

	this->_server.treat_command(this, this->_message_buffer);

	this->_message_buffer.clear();
}

bool Client::is_logged() const
{
	return this->_has_logged;
}

void Client::send(const std::string& data)
{
	this->_response_queue.push(data + "\n");
	this->_server.ask_pollfd_update();
}

size_t	Client::response_queue_size() const
{
	return this->_response_queue.size();
}

std::string Client::response_queue_pop()
{
	std::string last = this->_response_queue.front();
	this->_response_queue.pop();
	return last;
}

void Client::set_logged()
{
	this->_has_logged = true;
}

void Client::set_nickname(const std::string& nick)
{
	if (this->_nickname.empty())
	{
		std::cout << INFO << this->_address << ":" << this->_port << " changed their nickname to '" << nick << "'" << std::endl;
	}
	else
	{
		std::cout << INFO << this->_nickname << " changed their nickname to '" << nick << "'" << std::endl;
	}
	this->_nickname = nick;
}

void Client::set_username(const std::string& user)
{
	this->_username = user;
}

const std::string& Client::nickname() const
{
	return this->_nickname;
}

const std::string& Client::username() const
{
	return this->_username;
}

bool Client::is_registered() const
{
	return this->is_logged() && !this->_nickname.empty() && !this->_username.empty();
}

}
