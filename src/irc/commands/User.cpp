#include "irc/Command.hpp"
#include "irc/Client.hpp"
#include "irc/Errors.hpp"
#include "irc/Irc.hpp"

#include <iostream>

namespace ft
{
namespace cmd
{

User::User(Server& server)
	: Command(server, "USER", "<username> 0 * <realname>: Sets username")
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

	if (arguments[0].empty())
	{
		throw ERR_NEEDMOREPARAMS(this->_name);
	}

	if (arguments[0].size() > SERVER_USERLEN)
	{
		arguments[0] = arguments[0].substr(0, SERVER_USERLEN);
	}

	sender->set_username(arguments[0]);
	if (!sender->nickname().empty()) {
		this->_server.successfully_registered(sender->nickname(), sender);
	}
}

}
}
