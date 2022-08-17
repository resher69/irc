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

Notice::Notice(Server& server)
	: Command(server, "NOTICE", "<target>{,<target>} <text to be sent>: Send a notice")
{	}

void Notice::execute(Client *sender, const std::string& args)
{
	if (!sender->is_logged()) {
		throw ERR_NOLOGIN;
	}
	if (!sender->is_registered()) {
		throw ERR_NOTREGISTERED;
	}

	std::vector<std::string> arguments;
	this->parse_arguments(&args[0], arguments);

	if (arguments.size() < 2 || arguments[0].empty()) {
		throw ERR_NEEDMOREPARAMS(this->_name);
	}

	std::string parsed;
	std::stringstream stream(arguments[0]);

	while (std::getline(stream, parsed, ',')) {
		if (parsed.empty()) {
			continue ;
		}
		if (parsed[0] == '#') { // Channel
			try {
				Channel *channel = this->_server.get_channel_with_name(parsed);
				channel->dispatch_message(sender, NOTICE(sender->nickname(), sender->username(), parsed, arguments[1]));
			} catch (std::string& e) {
				sender->send(e);
			}
		} else { // Client
			try {
				Client *receiver = this->_server.get_from_nick(parsed);
				if (!receiver) {
					throw ERR_NOSUCHNICK(sender->nickname(), parsed);
				}
				receiver->send(NOTICE(sender->nickname(), sender->username(), parsed, arguments[1]));
			} catch (std::string& e) {
				sender->send(e);
			}
		}
	}
}

}
}
