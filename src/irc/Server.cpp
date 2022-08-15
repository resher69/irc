#include <iostream>
#include <stdexcept>
#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Command.hpp"
#include "Colors.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sstream>

namespace ft
{

Server::Server(int port, const std::string& password)
	:	_password(password), _clients(), _commands(), _should_update_pollfds(false)
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

	this->setup_commands();
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

	for (	std::map<std::string, Command *>::const_iterator it = this->_commands.begin();
			it != this->_commands.end();
			++it	)
	{
		delete it->second;
	}
}

void Server::update()
{
	this->handle_disconnects();
	this->handle_new_connections();
	if (this->_should_update_pollfds)
	{
		this->_pollfds.clear();
		for (	std::vector<Client *>::const_iterator it = this->_clients.begin();
				it != this->_clients.end();
				++it	)
		{
			int events = POLLIN | POLLHUP;
			if ((*it)->response_queue_size() > 0)
			{
				events |= POLLOUT;
			}
			this->_pollfds.push_back((pollfd){.fd = (*it)->socket(), .events = events, .revents = 0});
		}
	}

	this->_should_update_pollfds = false;

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
		if (pit->revents & POLLHUP)
		{
			this->request_disconnect(*cit);
		}
		if (pit->revents & POLLOUT)
		{
			if ((*cit)->response_queue_size() == 0)
			{
				throw std::runtime_error("POLLOUT was set but response queue is empty");
			}
			this->handle_data_to_client(*cit);
		}
	}
}

bool Server::has_password() const
{
	return !this->_password.empty();
}

const std::string& Server::password() const
{
	return this->_password;
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
		this->ask_pollfd_update();
	}
}

void Server::request_disconnect(Client *client)
{
	std::vector<Client *>& lst = this->_to_disconnect;
	if (std::find(lst.begin(), lst.end(), client) == lst.end()) {
		lst.push_back(client);
	}
}

void Server::disconnect(Client *client)
{
	// TODO: remove from all channels
	client->disconnect();
	std::cout << INFO << client->address() << ":" << client->port() << " has disconnected" << std::endl;

	std::vector<Client *>& disc = this->_to_disconnect;
	std::vector<Client *>& clients = this->_clients;

	disc.erase(std::find(disc.begin(), disc.end(), client));
	clients.erase(std::find(clients.begin(), clients.end(), client));

	delete client;

	this->ask_pollfd_update();
}

void Server::handle_disconnects()
{
	std::vector<Client *>& lst = this->_to_disconnect;

	while (!lst.empty()) {
		this->disconnect(*lst.begin());
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
			std::cout << INFO << "Received partial command from " << client->address() << ":" << client->port() << std::endl;
			n = recv(client->socket(), buf, 1023, 0);
			client->append_to_buffer(buf);
		}
	}
	else
	{
		this->request_disconnect(client);
	}
}

void Server::setup_commands()
{
	this->_commands["QUIT"] = new cmd::Quit(*this);
	this->_commands["PASS"] = new cmd::Pass(*this);
	this->_commands["NICK"] = new cmd::Nick(*this);
	this->_commands["USER"] = new cmd::User(*this);
}

void Server::treat_command(Client *sender, const std::string& command)
{
	std::string command_name;
	std::string args;
	Command *cmd = NULL;

	try {
		args = this->parse_command_name(command, command_name);
		std::transform(command_name.begin(), command_name.end(), command_name.begin(), ::toupper);
		cmd = this->_commands.at(command_name);
	} catch (const std::string& err) {
		std::cerr << ERROR << "Client::parse_command(): " << err << std::endl;
	} catch (const std::exception& err) {
		(void)err;
		if (command_name != "CAP" && command_name != "WHO" && command_name != "USERHOST")
		{
			std::cerr << ERROR << "Client::treat_command(): No such command '" << command_name << "'" << std::endl;
		}
	}

	if (cmd != NULL)
	{
		try {
			cmd->execute(sender, args);
		} catch (const std::string& err) {
			std::cerr << ERROR << command_name << ": " << err << std::endl;
			sender->send(err);
		}
	}
}

const char *Server::parse_command_name(const std::string& command, std::string& command_name)
{
	command_name.clear();
	const char *cmd = &command[0];

	while (*cmd == ' ') {
		++cmd;
	}
	if (!*cmd) {
		throw std::string("Command cannot be empty");
	}

	while (*cmd && *cmd != ' ') {
		if (*cmd == ':') {
			throw std::string("Command name cannot have ':' in it");
		}
		command_name += *cmd;
		++cmd;
	}
	while (*cmd && *cmd == ' ') {
		++cmd;
	}
	return cmd;
}

void Server::ask_pollfd_update()
{
	this->_should_update_pollfds = true;
}

void Server::handle_data_to_client(Client *client)
{
	std::string message = client->response_queue_pop();
	if (::send(client->socket(), &message[0], message.size(), 0) < 0)
	{
		this->request_disconnect(client);
	}
	if (client->response_queue_size() == 0)
	{
		this->ask_pollfd_update();
	}
}

Client *Server::get_from_nick(const std::string& nick)
{
	for (size_t i = 0; i < this->_clients.size(); ++i)
	{
		if (this->_clients[i]->nickname() == nick)
			return this->_clients[i];
	}
	return NULL;
}

void	Server::successfully_registered(const std::string& nick, Client *user)
{
	std::stringstream client_count;
	client_count << this->_clients.size();

	std::stringstream channel_count;
	channel_count << 0; // TODO: channel count

	user->send(RPL_WELCOME(nick, user->username()));
	user->send(RPL_YOURHOST(nick));
	user->send(RPL_BOUNCE(nick));
	user->send(RPL_LUSERCLIENT(nick, client_count.str()));
	user->send(RPL_LUSERCHANNELS(nick, channel_count.str()));
}

}
