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

Kick::Kick(Server& server)
	: Command(server, "KICK", "<channel> <user>{,<user>} [<comment>]: Kick a user from the channel (Channel operator only)")
{	}

void Kick::execute(Client *sender, const std::string& args)
{
	if (!sender->is_logged()) {
		throw ERR_NOLOGIN;
	}
	if (!sender->is_registered()) {
		throw ERR_NOTREGISTERED;
	}

	std::vector<std::string> arguments;
	this->parse_arguments(&args[0], arguments);

	if (arguments.size() < 2) {
		throw ERR_NEEDMOREPARAMS(this->_name);
	}

	Channel *channel = this->_server.get_channel_with_name(arguments[0]);

	if (!channel->is_operator(sender)) {
		throw ERR_CHANOPRIVSNEEDED(arguments[0]);
	}

	std::string comment;
	if (arguments.size() > 2) {
		comment = arguments[2];
	}

	std::string parsed;
	std::stringstream stream(arguments[1]);

	while (std::getline(stream, parsed, ',')) {
		Client *to_kick = channel->get_from_nick(parsed);
		if (!to_kick) {
			sender->send(ERR_NOSUCHNICK(sender->nickname(), parsed));
			continue ;
		}

		channel->dispatch_message(NULL, KICK(sender->nickname(), sender->username(), arguments[0], to_kick->nickname(), comment));
		channel->remove_client(to_kick, comment);
	}
}

}
}
