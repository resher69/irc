#include <iostream>
#include <stdexcept>
#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "Colors.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

namespace ft
{

Server::Server(int port, const std::string& password)
	: _password(password), _clients(), _should_update_pollfds(false)
{
	this->_socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (this->_socket < 0)
	{
		throw std::runtime_error(strerror(errno));
	}

	int opt = 1;
	if (::setsockopt(this->_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		throw std::runtime_error(strerror(errno));
	}

	if (::fcntl(this->_socket, F_SETFL, O_NONBLOCK) < 0)
	{
		throw std::runtime_error(strerror(errno));
	}

	struct sockaddr_in addr;

	std::memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (::bind(this->_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		throw std::runtime_error(strerror(errno));
	}

	if (::listen(this->_socket, 99999) < 0)
	{
		throw std::runtime_error(strerror(errno));
	}

	this->_clients.reserve(10);

	std::cout << INFO << "Listening on port " << port << std::endl;
}

Server::~Server()
{
	if (this->_socket >= 0)
	{
		close(this->_socket);
	}

	for (	std::vector<Client *>::const_iterator it = this->_clients.begin();
			it != this->_clients.end();
			++it	)
	{
		delete *it;
	}
}

void Server::update()
{
	this->handle_new_connections();
	if (this->_should_update_pollfds)
	{
		this->_pollfds.clear();
		for (	std::vector<Client *>::const_iterator it = this->_clients.begin();
				it != this->_clients.end();
				++it	)
		{
			// TODO: POLLUP
			this->_pollfds.push_back((pollfd){.fd = (*it)->socket(), .events = POLLIN | POLLHUP, .revents = 0});
		}
	}

	if (::poll(&this->_pollfds[0], this->_pollfds.size(), 30) < 0)
	{
		throw std::runtime_error(strerror(errno));
	}

	std::vector<Client *>::const_iterator cit = this->_clients.begin();
	for (	std::vector<pollfd>::const_iterator pit = this->_pollfds.begin();
				pit != this->_pollfds.end();
				++pit, ++cit	)
	{
		if (pit->revents & POLLIN)
		{
			this->handle_data_from_client(*cit);
		}
	}
}

bool Server::has_password() const
{
	return !this->_password.empty();
}

void Server::handle_new_connections()
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int client_fd = ::accept(this->_socket, (struct sockaddr *)&addr, &addrlen);
	if (client_fd >= 0)
	{
		Client *client = new Client(*this, client_fd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		this->_clients.push_back(client);
		this->_should_update_pollfds = true;
	}
}

void Server::handle_data_from_client(Client *client)
{
	char buf[1024] = { 0 };
	int n = recv(client->socket(), buf, 1023, MSG_PEEK);
	if (n > 0)
	{
		if (char *pos = std::strstr(buf, "\n"))
		{
			size_t len = pos - (char *)buf;
			std::memset(buf, 0, 1023);
			n = recv(client->socket(), buf, len + 1, 0);
			client->append_to_buffer(buf);
			client->treat_command();
		}
		else
		{
			n = recv(client->socket(), buf, 1023, 0);
			client->append_to_buffer(buf);
		}
	}
	else
	{
		throw std::runtime_error("TODO: Handle disconnect on read() <= 0");
	}
}

}
