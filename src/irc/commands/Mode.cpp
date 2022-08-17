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

Mode::Mode(Server& server)
	: Command(server, "MODE", "<channel> [<modestring>] [<param>]")
{	}

void Mode::execute(Client *sender, const std::string& args)
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

	if (arguments.size() == 1) {
		std::string modes;

		if (channel->mode() & MODE_K) {
			modes += "k";
		}
		if (channel->mode() & MODE_T) {
			modes += "t";
		}

		sender->send(RPL_CHANNELMODEIS(sender->nickname(), channel->name(), modes));
		return ;
	}

	if (arguments.size() == 2) {
		if (arguments[1].size() < 2) {
			throw ERR_NEEDMOREPARAMS(this->_name);
		}

		const std::vector<Client *>& list = channel->ban_list();

		if (arguments[1][0] == '+') {
			switch (arguments[1][1]) {
				case 'b': // 'MODE <channel> +b' -> return banlist to sender
					for (size_t i = 0; i < list.size(); ++i) {
						sender->send(RPL_BANLIST(sender->nickname(), channel->name(), list[i]->nickname()));
					}
					sender->send(RPL_ENDOFBANLIST(sender->nickname(), channel->name()));
					break;
				case 't': // 'MODE <channel> +t' -> set topic protection
					if (!channel->is_operator(sender)) {
						throw ERR_CHANOPRIVSNEEDED(channel->name());
					}
					channel->set_mode(MODE_T);
					channel->dispatch_message(NULL, MODE_TOPICPROTECTED(sender->nickname(), channel->name()));
					break ;
				case 'o': // 'MODE <channel> +o' -> invalid
					throw ERR_NEEDMOREPARAMS(this->_name);
					break ;
				case 'k': // 'MODE <channel> +k' -> invalid
					throw ERR_NEEDMOREPARAMS(this->_name);
					break ;
			}
			return ;
		} else if (arguments[1][0] == '-') {
			switch (arguments[1][1]) {
				case 'b': // 'MODE <channel> -b' -> invalid
					throw ERR_NEEDMOREPARAMS(this->_name);
					break ;
				case 't': // 'MODE <channel> -t' -> remove topic protection
					if (!channel->is_operator(sender)) {
						throw ERR_CHANOPRIVSNEEDED(channel->name());
					}
					channel->unset_mode(MODE_T);
					channel->dispatch_message(NULL, MODE_TOPICUNPROTECTED(sender->nickname(), channel->name()));
					break ;
				case 'o': // 'MODE <channel> -o' -> invalid
					throw ERR_NEEDMOREPARAMS(this->_name);
					break ;
				case 'k': // 'MODE <channel> -k' -> unset password
					channel->unset_mode(MODE_K);
					break ;
			}
			return ;
		} else {
			throw std::string("Wrong format");
		}
	}

	Client *client = NULL;

	if (arguments.size() > 2) {
		if (arguments[1][0] == '+') {
			switch (arguments[1][1]) {
				case 'b': // 'MODE <channel> +b <nickname>' -> ban user
					if (!channel->is_operator(sender)) {
						throw ERR_CHANOPRIVSNEEDED(channel->name());
					}
					client = channel->get_from_nick(arguments[2]);
					if (client == NULL) {
						throw ERR_NOSUCHNICK(sender->nickname(), arguments[2]);
					}
					channel->ban(client);
					channel->dispatch_message(NULL, MODE_BAN(sender->nickname(), channel->name(), arguments[2]));
					break;
				case 't': // 'MODE <channel> +t <params>' -> invalid
					throw ERR_NEEDMOREPARAMS(this->_name);
					break ;
				case 'o': // 'MODE <channel> +o <user>' -> promote user as operator
					if (!channel->is_operator(sender)) {
						throw ERR_CHANOPRIVSNEEDED(channel->name());
					}
					client = channel->get_from_nick(arguments[2]);
					if (client == NULL) {
						throw ERR_NOSUCHNICK(sender->nickname(), arguments[2]);
					}
					channel->set_operator(client);
					channel->dispatch_message(NULL, MODE_OP(sender->nickname(), channel->name(), arguments[2]));
					break ;
				case 'k': // 'MODE <channel> +k <key>' -> set password on channel and set key to <key>
					if (!channel->is_operator(sender)) {
						throw ERR_CHANOPRIVSNEEDED(channel->name());
					}
					channel->set_mode(MODE_K);
					channel->set_key(arguments[2]);
					break ;
			}
			return ;
		} else if (arguments[1][0] == '-') {
			switch (arguments[1][1]) {
				case 'b': // 'MODE <channel> -b <user>' -> unban user
					if (!channel->is_operator(sender)) {
						throw ERR_CHANOPRIVSNEEDED(channel->name());
					}
					client = channel->get_from_nick(arguments[2]);
					if (client == NULL) {
						throw ERR_NOSUCHNICK(sender->nickname(), arguments[2]);
					}
					channel->dispatch_message(NULL, MODE_UNBAN(sender->nickname(), channel->name(), arguments[2]));
					channel->unban(client);
					break ;
				case 't': // 'MODE <channel> -t <params>' -> invalid
					throw ERR_NEEDMOREPARAMS(this->_name);
					break ;
				case 'o': // 'MODE <channel> -o <user>' -> deop user
					if (!channel->is_operator(sender)) {
						throw ERR_CHANOPRIVSNEEDED(channel->name());
					}
					client = channel->get_from_nick(arguments[2]);
					if (client == NULL) {
						throw ERR_NOSUCHNICK(sender->nickname(), arguments[2]);
					}
					channel->unset_operator(client);
					channel->dispatch_message(NULL, MODE_DEOP(sender->nickname(), channel->name(), arguments[2]));
					break ;
				case 'k': // 'MODE <channel> -k <key>' -> invalid
					throw ERR_NEEDMOREPARAMS(this->_name);
					break ;
			}
			return ;
		} else {
			throw std::string("Wrong format");
		}
	}
}

}
}
