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

Topic::Topic(Server& server)
	: Command(server, "TOPIC", "Show a channel topic")
{	}

void Topic::execute(Client *sender, const std::string& args)
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

	Channel *channel = this->_server.get_channel_with_name(arguments[0]);

	if (arguments.size() > 1) {
		if (channel->mode() & MODE_T && !channel->is_operator(sender)) {
			throw ERR_CHANOPRIVSNEEDED(channel->name());
		}
		channel->set_topic(arguments[1]);
		channel->dispatch_message(NULL, TOPIC(sender->nickname(), sender->username(), arguments[0], arguments[1]));
	} else {
		sender->send(RPL_TOPIC(sender->nickname(), arguments[0], channel->topic()));
	}
}

}
}
