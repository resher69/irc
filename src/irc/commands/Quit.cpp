#include "irc/Command.hpp"
#include "irc/Client.hpp"
#include "irc/Errors.hpp"

namespace ft
{
namespace cmd
{

Quit::Quit(Server& server)
	: Command(server, "QUIT", "QUIT <reason>: Leaves the server with a message")
{	}

void Quit::execute(Client *sender, const std::string& args)
{
	if (args.size() == 0)
	{
		throw ERR_NEEDMOREPARAMS(this->_name);
	}
	if (!sender->is_logged())
	{
		throw ERR_NOLOGIN;
	}

	this->_server.request_disconnect(sender);
}

}
}
