#pragma once

#include <string>
#include <vector>

typedef struct pollfd pollfd;

namespace ft
{
/* Classe incomplete */
class Client;

class Server
{
private:
	int _socket;
	std::string _password;

	std::vector<Client *> _clients;
	std::vector<pollfd>	 _pollfds;

	bool _should_update_pollfds;

public:
	Server(int port, const std::string& password);
	~Server();

	void update();
	bool has_password() const;

private:
	void handle_new_connections();
	void handle_data_from_client(Client *client);
};

}
