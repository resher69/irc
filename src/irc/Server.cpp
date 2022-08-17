#include <iostream>
#include <stdexcept>
#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/Channel.hpp"
#include "irc/Errors.hpp"
#include "Colors.hpp"
#include "utils.hpp"

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
	:	_password(password), _clients(), _channels(), _commands(), _should_update_pollfds(false)
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

	std::cout << PRINT_INFO << "Listening on port " << port << std::endl;
	if (this->has_password()) {
		std::cout << PRINT_INFO << "Password: '" << password << "'" << std::endl;
	}

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

	for (	std::map<std::string, Channel *>::const_iterator it = this->_channels.begin();
			it != this->_channels.end();
			++it	)
	{
		delete it->second;
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
	std::vector<Channel *> channels = this->get_channels_with_client(client);

	for (size_t i = 0; i < channels.size(); ++i) {
		channels[i]->remove_client(client, "Disconnected");
	}

	client->disconnect();
	std::cout << PRINT_INFO << client->address() << ":" << client->port() << " has disconnected" << std::endl;

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
			std::cout << PRINT_INFO << "Received partial command from " << client->address() << ":" << client->port() << std::endl;
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
	this->_commands["PING"] = new cmd::Ping(*this);
	this->_commands["JOIN"] = new cmd::Join(*this);
	this->_commands["PRIVMSG"] = new cmd::Privmsg(*this);
	this->_commands["HELP"] = new cmd::Help(*this);
	this->_commands["LIST"] = new cmd::List(*this);
	this->_commands["NOTICE"] = new cmd::Notice(*this);
	this->_commands["TOPIC"] = new cmd::Topic(*this);
	this->_commands["KICK"] = new cmd::Kick(*this);
	this->_commands["PART"] = new cmd::Part(*this);
	this->_commands["MODE"] = new cmd::Mode(*this);
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
		std::cerr << PRINT_ERROR << "Client::parse_command(): " << err << std::endl;
	} catch (const std::exception& err) {
		(void)err;
		if (command_name != "CAP" && command_name != "WHO" && command_name != "USERHOST")
		{
			sender->send(ERR_UNKNOWNCOMMAND(command_name));
			std::cerr << PRINT_ERROR << "Client::treat_command(): No such command '" << command_name << "'" << std::endl;
		}
	}

	if (cmd != NULL)
	{
		try {
			cmd->execute(sender, args);
		} catch (const std::string& err) {
			std::cerr << PRINT_ERROR << command_name << ": " << err << std::endl;
			sender->send(err);
		}
	}
}

void Server::dispatch_message(const std::string& message) const
{
	for (std::vector<Client *>::const_iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
		(*it)->send(message);
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
	std::cout << PRINT_SERVER << client->address() << ":" << client->port() << ": " << message << std::flush;
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
	user->send(RPL_WELCOME(nick, user->username()));
	user->send(RPL_YOURHOST(nick));
	user->send(RPL_BOUNCE(nick));
	user->send(RPL_LUSERCLIENT(nick, convert_string(this->_clients.size())));
	user->send(RPL_LUSERCHANNELS(nick, convert_string(this->_channels.size())));
}

void Server::create_channel(Client *creator, const std::string& name, const std::string& key)
{
	if (key.empty()) {
		std::cout << PRINT_INFO << YEL << "Creating channel '" << COLOR_RESET << name << YEL << "'" << COLOR_RESET << std::endl;
		this->_channels[name] = new Channel(creator, name, *this);
	} else {
		std::cout << PRINT_INFO << YEL << "Creating channel '" << COLOR_RESET << name << YEL << "' with key " << CYN << key << COLOR_RESET << std::endl;
		this->_channels[name] = new Channel(creator, name, key, *this);
	}
}

void Server::remove_channel(const std::string& name)
{
	try {
		std::map<std::string, Channel *>::iterator channel = this->_channels.find(name);
		delete channel->second;
		channel->second = NULL;
		this->_channels.erase(channel);
	}
	catch (const std::exception&)
	{	}
}

void Server::join_channel(Client *client, const std::string& name, const std::string& key)
{
	try {
		Channel *channel = this->_channels.at(name);

		if (channel->mode() & MODE_K) {
			if (!key.empty() && channel->is_right_key(key)) {
				channel->add_client(client);
			} else {
				throw ERR_BADCHANNELKEY(name);
			}
		} else {
			channel->add_client(client);
		}
	} catch (const std::exception&) {
		this->create_channel(client, name, key);
	}
}

size_t Server::channel_count() const
{
	return this->_channels.size();
}

Channel *Server::get_channel_with_name(const std::string& name)
{
	try {
		return this->_channels.at(name);
	} catch (const std::exception&) {
		throw ERR_NOSUCHCHANNEL(name);
	}
}

std::vector<Channel *>	Server::get_channels() const
{
	std::vector<Channel *> ret;

	for (std::map<std::string, Channel *>::const_iterator it = this->_channels.begin(); it != this->_channels.end(); ++it) {
		ret.push_back(it->second);
	}
	return ret;
}

std::vector<Channel *>	Server::get_channels_with_client(Client *client) const
{
	std::vector<Channel *> ret;

	for (std::map<std::string, Channel *>::const_iterator it = this->_channels.begin(); it != this->_channels.end(); ++it) {
		if (it->second->client_exists(client)) {
			ret.push_back(it->second);
		}
	}
	return ret;
}

const std::map<std::string, Command *>& Server::get_commands() const
{
	return this->_commands;
}

}
