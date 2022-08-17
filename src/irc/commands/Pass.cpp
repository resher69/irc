#include "irc/Command.hpp"
#include "irc/Client.hpp"
#include "irc/Errors.hpp"

#include <iostream>

namespace ft
{
namespace cmd
{

Pass::Pass(Server& server)
	: Command(server, "PASS", "<password>: Log in to the server")
{	}

void Pass::execute(Client *sender, const std::string& args)
{
	if (sender->is_logged())
	{
		throw ERR_ALREADYREGISTERED;
	}

	std::vector<std::string> arguments;

	this->parse_arguments(&args[0], arguments);
	if (arguments.size() == 0)
	{
		throw ERR_NEEDMOREPARAMS(_name);
	}

	const std::string& password = arguments[0];
	if (password == this->_server.password())
	{
		sender->set_logged();
	}
	else
	{
		throw ERR_PASSWDMISMATCH;
	}
}

}
}
