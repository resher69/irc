#include "irc/Command.hpp"
#include "irc/Client.hpp"
#include "irc/Errors.hpp"

namespace ft
{
namespace cmd
{

Ping::Ping(Server& server)
	: Command(server, "PING", "<message>: Returns back PONG <message>")
{	}

void Ping::execute(Client *sender, const std::string& args)
{
	if (!sender->is_logged()) {
		throw ERR_NOLOGIN;
	}
	if (!sender->is_registered()) {
		throw ERR_NOTREGISTERED;
	}

	std::vector<std::string> arguments;
	this->parse_arguments(&args[0], arguments);

	if (arguments.size() == 0) {
		throw ERR_NEEDMOREPARAMS(this->_name);
	}

	sender->send("PONG " + arguments[0]);
}

}
}
