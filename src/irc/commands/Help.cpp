#include "irc/Command.hpp"
#include "irc/Client.hpp"
#include "irc/Channel.hpp"
#include "irc/Errors.hpp"
#include "irc/Replies.hpp"
#include "irc/Irc.hpp"

#include <iostream>
#include <sstream>

namespace ft
{
namespace cmd
{

Help::Help(Server& server)
	: Command(server, "HELP", "Describes commands")
{	}

void Help::execute(Client *sender, const std::string&)
{
	if (!sender->is_logged()) {
		throw ERR_NOLOGIN;
	}
	if (!sender->is_registered()) {
		throw ERR_NOTREGISTERED;
	}


	for (std::map<std::string, Command *>::const_iterator it = this->_server.get_commands().begin(); it != this->_server.get_commands().end(); ++it) {
		sender->send(HELP(sender->nickname(), sender->username(), it->second->name() + " " + it->second->description()));
	}
}

}
}
