#include "irc/Channel.hpp"
#include "irc/Replies.hpp"
#include "irc/Errors.hpp"
#include "Colors.hpp"

#include <algorithm>
#include <iostream>

namespace ft
{

Channel::Channel(Client *creator, const std::string& name, Server& server)
	: _name(name), _mode(MODE_T), _topic(""), _clients(), _server(server), _banlist()
{
	this->add_client(creator);
	this->set_operator(creator);
}

Channel::Channel(Client *creator, const std::string& name, const std::string& key, Server& server)
	: _name(name), _mode(MODE_T | MODE_K), _key(key), _topic(""), _clients(), _server(server), _banlist()
{
	this->add_client(creator);
	this->set_operator(creator);
}

Channel::~Channel()
{

}

bool Channel::is_banned(Client *client) const
{
	return std::find(this->_banlist.begin(), this->_banlist.end(), client) != this->_banlist.end();
}

const std::vector<Client *>&	Channel::ban_list() const
{
	return this->_banlist;
}

void Channel::add_client(Client *client)
{
	if (this->client_exists(client)) {
		return ;
	}
	this->_clients.push_back((ClientInfo) { .client = client, .is_operator = false });

	std::string nick = client->nickname();

	if (this->_mode & MODE_T) {
		client->send(RPL_TOPIC(nick, this->_name, this->_topic));
	} else {
		client->send(RPL_NOTOPIC(nick, this->_name));
	}

	client->send(RPL_NAMREPLY(nick, this->_name, this->list_client_nicks()));
	client->send(RPL_ENDOFNAMES(nick, this->_name));

	this->dispatch_message(NULL, JOIN(nick, client->username(), this->_name));

	this->update_clients();
}

void Channel::remove_client(Client *client, const std::string& reason)
{
	this->dispatch_message(NULL, PART(client->nickname(), client->username(), this->_name, reason));
	if (!this->client_exists(client)) {
		return ;
	}
	this->unset_operator(client);
	std::vector<ClientInfo>::iterator it = this->find_client(client);

	if (it != this->_clients.end()) {
		this->_clients.erase(it);
	}

	this->update_clients();

	if (this->_clients.empty()) {
		std::cout << YEL "Removing channel '" << COLOR_RESET << this->_name << YEL << "'" << COLOR_RESET << std::endl;
		this->_server.remove_channel(this->_name);
	}
}

const std::string& Channel::name() const
{
	return this->_name;
}

short Channel::mode() const
{
	return this->_mode;
}

void Channel::set_mode(short mode)
{
	this->_mode |= mode;
}

void Channel::unset_mode(short mode)
{
	this->_mode &= ~mode;
}

const std::string& Channel::key() const
{
	return this->_key;
}

bool Channel::is_right_key(const std::string& key) const
{
	return key == this->_key;
}

const std::string& Channel::topic() const
{
	return this->_topic;
}

void Channel::set_topic(const std::string& topic)
{
	if (topic.size() > SERVER_TOPICLEN) {
		throw ERR_NEEDMOREPARAMS("TOPIC");
	}
	this->_topic = topic;
}

void Channel::ban(Client *client)
{
	if (std::find(this->_banlist.begin(), this->_banlist.end(), client) == this->_banlist.end()) {
		this->_banlist.push_back(client);
	}
}

void Channel::unban(Client *client)
{
	std::vector<Client *>::iterator it = std::find(this->_banlist.begin(), this->_banlist.end(), client);
	if (it != this->_banlist.end()) {
		this->_banlist.erase(it);
	}
}

bool Channel::client_exists(Client *client) const
{
	return this->find_client(client) != this->_clients.end();
}

const std::string Channel::list_client_nicks() const
{
	std::string clients;

	for (std::vector<ClientInfo>::const_iterator it = this->_clients.begin(); it != this->_clients.end(); it++) {
		if (it->client->is_registered()) {
			if (this->is_operator(it->client)) {
				clients += "@" + it->client->nickname() + " ";
			} else {
				clients += it->client->nickname() + " ";
			}
		}
	}
	return clients;
}

void Channel::update_clients() const
{
	for (std::vector<ClientInfo>::const_iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
		it->client->send(RPL_NAMREPLY(it->client->nickname(), this->_name, this->list_client_nicks()));
		it->client->send(RPL_ENDOFNAMES(it->client->nickname(), this->_name));
	}
}

size_t Channel::client_count() const
{
	return this->_clients.size();
}

void Channel::set_operator(Client *client)
{
	this->find_client_info(client).is_operator = true;
}

void Channel::unset_operator(Client *client)
{
	this->find_client_info(client).is_operator = false;
}

bool Channel::is_operator(Client *client) const
{
	return this->find_client_info(client).is_operator;
}

void Channel::dispatch_message(Client *client, const std::string& message) const
{
	for (std::vector<ClientInfo>::const_iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
		// Do not send the message to the sender
		if (client && it->client->socket() == client->socket()) {
			continue ;
		}
		it->client->send(message);
	}
}

std::vector<Channel::ClientInfo>::iterator		Channel::find_client(Client *client)
{
	for (std::vector<ClientInfo>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
		if (it->client == client) {
			return it;
		}
	}
	return this->_clients.end();
}

std::vector<Channel::ClientInfo>::const_iterator	Channel::find_client(Client *client) const
{
	for (std::vector<Channel::ClientInfo>::const_iterator it = this->_clients.begin(); it != this->_clients.end(); ++it) {
		if (it->client == client) {
			return it;
		}
	}
	return this->_clients.end();
}

Channel::ClientInfo& Channel::find_client_info(Client *client)
{
	std::vector<Channel::ClientInfo>::iterator it = this->find_client(client);
	return *it;
}

const Channel::ClientInfo& Channel::find_client_info(Client *client) const
{
	std::vector<Channel::ClientInfo>::const_iterator it = this->find_client(client);
	return *it;
}

Client *Channel::get_from_nick(const std::string& nick)
{
	for (size_t i = 0; i < this->_clients.size(); ++i)
	{
		if (this->_clients[i].client->nickname() == nick)
			return this->_clients[i].client;
	}
	return NULL;
}

}
