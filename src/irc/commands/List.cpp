#include "irc/Command.hpp"
#include "irc/Client.hpp"
#include "irc/Channel.hpp"
#include "irc/Errors.hpp"
#include "irc/Replies.hpp"
#include "irc/Irc.hpp"
#include "utils.hpp"

#include <iostream>
#include <sstream>

namespace ft
{
namespace cmd
{

List::List(Server& server)
	: Command(server, "LIST", "List channels")
{	}

void List::execute(Client *sender, const std::string&)
{
	if (!sender->is_logged()) {
		throw ERR_NOLOGIN;
	}
	if (!sender->is_registered()) {
		throw ERR_NOTREGISTERED;
	}

	std::vector<Channel *> channels = this->_server.get_channels();

	sender->send(RPL_LISTSTART(sender->nickname()));
	for (std::vector<Channel *>::iterator channel = channels.begin(); channel != channels.end(); ++channel) {
		sender->send(RPL_LIST(sender->nickname(), (*channel)->name(), (*channel)->client_count(), (*channel)->topic()));
	}
	sender->send(RPL_LISTEND);
}

}
}
