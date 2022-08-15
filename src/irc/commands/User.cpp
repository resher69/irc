#include "irc/Command.hpp"
#include "irc/Client.hpp"
#include "irc/Errors.hpp"

#include <iostream>

namespace ft
{
namespace cmd
{

User::User(Server& server)
	: Command(server, "USER", "USER <username> 0 <unused> <realname>: Sets username")
{	}

void User::execute(Client *sender, const std::string& args)
{
	if (!sender->is_logged())
	{
		throw ERR_NOLOGIN;
	}
	if (sender->is_registered())
	{
		throw ERR_ALREADYREGISTERED;
	}

	std::vector<std::string> arguments;
	this->parse_arguments(&args[0], arguments);

	if (arguments.size() < 4)
	{
		throw ERR_NEEDMOREPARAMS(this->_name);
	}

	sender->set_username(arguments[0]);

	// TODO: successfully_registered
}

}
}
