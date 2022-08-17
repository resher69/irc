#include "irc/Command.hpp"
#include "irc/Client.hpp"
#include "irc/Errors.hpp"
#include "irc/Irc.hpp"

#include <iostream>
#include <sstream>

namespace ft
{
namespace cmd
{

Join::Join(Server& server)
	: Command(server, "JOIN", "<channel>{,<channel>} [<key>{,<key>}]")
{	}

void Join::execute(Client *sender, const std::string& args)
{
	if (!sender->is_logged())
	{
		throw ERR_NOLOGIN;
	}
	if (!sender->is_registered())
	{
		throw ERR_NOTREGISTERED;
	}

	std::vector<std::string> arguments;
	this->parse_arguments(&args[0], arguments);

	if (arguments.size() == 0)
	{
		throw ERR_NEEDMOREPARAMS(this->_name);
	}

	if (arguments.size() == 1 && arguments[0] == "0") {
		// TODO: Leave every channel
	}

	if (arguments[0].empty())
	{
		throw ERR_NEEDMOREPARAMS(this->_name);
	}

	std::vector<Join::ChannelInfo> channels;
	this->parse_infos(arguments[0], (arguments.size() > 1) ? arguments[1] : "", channels);

	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i].name[0] != '#' || channels[i].name.size() > SERVER_CHANNELLEN) {
			sender->send(ERR_NOSUCHCHANNEL(channels[i].name));
		} else {
			try {
				this->_server.join_channel(sender, channels[i].name, channels[i].key);
			} catch (const std::string& e) {
				sender->send(e);
			}
		}
	}
}

void Join::parse_infos	(	const std::string& channel_string,
							const std::string& key_string,
							std::vector<ChannelInfo>& channelinfos	)
{
	std::string parsed;
	std::stringstream channel_stream(channel_string);
	std::stringstream key_stream(key_string);

	std::vector<std::string> channels;
	while (std::getline(channel_stream, parsed, ',')) {
		channels.push_back(parsed);
	}

	std::vector<std::string> keys;
	while (std::getline(key_stream, parsed, ',')) {
		keys.push_back(parsed);
	}

	for (size_t i = 0; i < channels.size(); ++i) {
		channelinfos.push_back((Join::ChannelInfo){
			.name = channels[i],
			.key = (i < keys.size()) ? keys[i] : ""
		});
	}
}

}
}
