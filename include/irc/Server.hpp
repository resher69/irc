#pragma once

#include <string>
#include <vector>
#include <map>

typedef struct pollfd pollfd;

namespace ft
{
/* Classe incomplete */
class Client;
class Command;

class Server
{
private:
	int _socket;
	std::string _password;

	std::vector<Client *> _clients;
	std::vector<Client *> _to_disconnect;
	std::vector<pollfd>	 _pollfds;
	std::map<std::string, Command *> _commands;

	bool _should_update_pollfds;

public:
	Server(int port, const std::string& password);
	~Server();

	void update();
	bool has_password() const;

	void treat_command(Client *sender, const std::string& command);
	void request_disconnect(Client *client);

	void ask_pollfd_update();

	const std::string& password() const;

	Client *get_from_nick(const std::string& nick);
	void	successfully_registered(const std::string& nick, Client *user);

private:
	void setup_commands();

	void handle_new_connections();
	void handle_disconnects();
	void disconnect(Client *client);
	void handle_data_from_client(Client *client);
	void handle_data_to_client(Client *client);

	const char *parse_command_name(const std::string& command, std::string& command_name);
};

}
