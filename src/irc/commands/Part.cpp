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

Part::Part(Server& server)
	: Command(server, "PART", "<channel>{,<channel>} [<reason>]")
{	}

void Part::execute(Client *sender, const std::string& args)
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

	std::string comment;
	if (arguments.size() > 1) {
		comment = arguments[1];
	}

	std::string parsed;
	std::stringstream stream(arguments[0]);

	while (std::getline(stream, parsed, ',')) {
		try {
			Channel *channel = this->_server.get_channel_with_name(parsed);
			if (!channel->client_exists(sender)) {
				sender->send(ERR_NOTONCHANNEL(sender->nickname(), channel->name()));
				continue ;
			} else {
				channel->remove_client(sender, comment);
			}
		} catch (const std::string& e) {
			sender->send(e);
		}
	}
}

}
}
