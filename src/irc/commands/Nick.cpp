#include "irc/Command.hpp"
#include "irc/Client.hpp"
#include "irc/Errors.hpp"

namespace ft
{
namespace cmd
{

Nick::Nick(Server& server)
	: Command(server, "NICK", "<nickname>: Sets nickname")
{	}

void Nick::execute(Client *sender, const std::string& args)
{
	if (!sender->is_logged())
	{
		throw ERR_NOLOGIN;
	}

	std::vector<std::string> arguments;
	this->parse_arguments(&args[0], arguments);

	if (arguments.size() == 0)
	{
		throw ERR_NONICKNAMEGIVEN;
	}

	const std::string& nickname = arguments[0];

	if (nickname.size() > SERVER_NICKLEN)
	{
		throw ERR_ERRONEUSNICKNAME(sender->nickname());
	}

	for (size_t i = 0; i < nickname.size(); ++i)
	{
		if (!::isalnum(nickname[i]) && nickname[i] != '_')
		{
			throw ERR_ERRONEUSNICKNAME(sender->nickname());
		}
	}

	if (this->_server.get_from_nick(nickname) != NULL)
	{
		throw ERR_NICKNAMEINUSE(nickname);
	}

	if (sender->nickname().empty() && !sender->username().empty()) {
		this->_server.successfully_registered(nickname, sender);
	}

	sender->set_nickname(nickname);
}

}
}
