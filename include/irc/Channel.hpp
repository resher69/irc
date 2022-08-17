#pragma once

#include <string>
#include <vector>

#include "irc/Client.hpp"
#include "irc/Server.hpp"

#define MODE_T	(1 << 0)
#define MODE_K	(1 << 1)

namespace ft
{

class Channel
{
	private:
		struct ClientInfo {
			Client	*client;
			bool	is_operator;
		};

		std::string				_name;
		short					_mode;
		std::string				_key;
		std::string				_topic;
		std::vector<ClientInfo> _clients;
		Server&					_server;
		std::vector<Client *>	_banlist;

	public:
		Channel(Client *creator, const std::string& name, Server& server);
		Channel(Client *creator, const std::string& name, const std::string& key, Server& server);
		~Channel();

		const std::string& name() const;
		short mode() const;
		void set_mode(short mode);
		void unset_mode(short mode);

		const std::string& key() const;
		bool is_right_key(const std::string& key) const;

		const std::string& topic() const;
		void set_topic(const std::string& topic);

		void ban(Client *client);
		void unban(Client *client);
		bool is_banned(Client *client) const;
		const std::vector<Client *>&	ban_list() const;

		void add_client(Client *client);
		void remove_client(Client *client, const std::string& reason = "");
		bool client_exists(Client *client) const;
		const std::string list_client_nicks() const;
		void update_clients() const;
		size_t client_count() const;

		Client *get_from_nick(const std::string& nick);

		void set_operator(Client *client);
		void unset_operator(Client *client);
		bool is_operator(Client *client) const;

		void dispatch_message(Client *client, const std::string& message) const;

	private:
		std::vector<ClientInfo>::iterator		find_client(Client *client);
		std::vector<ClientInfo>::const_iterator	find_client(Client *client) const;
		ClientInfo&								find_client_info(Client *client);
		const ClientInfo&						find_client_info(Client *client) const;
};

}
