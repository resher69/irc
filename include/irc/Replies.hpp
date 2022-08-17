#pragma once

#include "Irc.hpp"

#define RPL_WELCOME(nick, username)								std::string(":") + SERVER_NAME + " 001 " + nick + " :Welcome to the Internet Relay Network " + username + "!~" + nick
#define RPL_YOURHOST(nick)										std::string(":") + SERVER_NAME + " 002 " + nick + " :Your host is " + SERVER_NAME + ", running version " + SERVER_VER
// #define RPL_CREATED(nick)									std::string(":") + SERVER_NAME + " 003 " + nick + " :This server has been started <date>"
// #define RPL_MYINFO(nick)										std::string(":") + SERVER_NAME + " 004 " + nick + " " + SERVER_NAME + " " + SERVER_VER + " abBcCFiIoqrRswx abehiIklmMnoOPqQrRstvVz"
#define RPL_BOUNCE(nick)										std::string(":") + SERVER_NAME + " 005 " + nick +	" RFC2812" \
																													" IRCD=" + SERVER_VER + \
																													" CHARSET=UTF-8" \
																													" CASEMAPPING=ascii" \
																													" PREFIX=" + SERVER_PREFIX + \
																													" CHANTYPES=#" \
																													" CHANMODES=kt" \
																													" CHANNELLEN=" + convert_string(SERVER_CHANNELLEN) + \
																													" NICKLEN=" + convert_string(SERVER_NICKLEN) + \
																													" USERLEN=" + convert_string(SERVER_USERLEN) + \
																													" TOPICLEN=" + convert_string(SERVER_TOPICLEN) + \
																													" MODES=1" \
																													" :are supported on this server"
#define RPL_LUSERCLIENT(nick, users_count)						std::string(":") + SERVER_NAME + " 251 " + nick + " :There are " + users_count + " users"
#define RPL_LUSERCHANNELS(nick, channels_count)					std::string(":") + SERVER_NAME + " 254 " + nick + " " + channels_count + " :channels formed"
// #define RPL_LUSERME(nick, users_count)						std::string(":") + SERVER_NAME + " 255 " + nick + " :I have " + users_count + " users"
#define RPL_LOCALUSERS(nick, users_count, users_max)			std::string(":") + SERVER_NAME + " 265 " + nick + " " + users_count + " " + users_max + " :Current users: " + users_count + ", Max: " + users_max
// #define RPL_GLOBALUSERS(nick, users_count, users_max) 		std::string(":") + SERVER_NAME + " 266 " + nick + "  1 :Current global users: 1, Max: 1"
// #define RPL_HCONNECTIONCOUNT(nick, )	    					std::string(":") + SERVER_NAME + " 250 " + nick + " :Highest connection count: 1 (2 connections received)"
#define RPL_LISTSTART(nick)										std::string(":") + SERVER_NAME + " 321 " + nick + " Channel :Users Name"
#define RPL_LIST(nick, channel_name, users_count, topic)		std::string(":") + SERVER_NAME + " 322 " + nick + " " + channel_name + " " + convert_string(users_count) + " :" + topic
#define RPL_LISTEND												std::string(":") + SERVER_NAME + " 323 " + ":End of /LIST"
// #define RPL_MOTDSTART(nick, )								std::string(":") + SERVER_NAME + " 375 " + nick + " :- " + SERVER_NAME + " message of the day"
// #define RPL_MOTD(nick, modt)									std::string(":") + SERVER_NAME + " 372 " + nick + " :- "+ modt
// #define RPL_ENDOFMOTD(nick, )								std::string(":") + SERVER_NAME + " 376 " + nick + " :End of MOTD command"
#define RPL_NAMREPLY(nick, channel_name, users_list)			std::string(":") + SERVER_NAME + " 353 " + nick + " = " + channel_name + " :" + users_list
#define RPL_ENDOFNAMES(nick, channel_name)						std::string(":") + SERVER_NAME + " 366 " + nick + " " + channel_name + " :End of NAMES list"
#define RPL_USERHOST(nick, username)							std::string(":") + SERVER_NAME + " 302 " + nick + " :" + username + "=+~" + nick + "@localhost"
#define RPL_ENDOFWHO(nick)										std::string(":") + SERVER_NAME + " 315 " + nick + " " + nick + " :End of WHO list"
#define RPL_CHANNELMODEIS(nick, channel_name, channel_mode)		std::string(":") + SERVER_NAME + " 324 " + nick + " " + channel_name + " +" + channel_mode
#define RPL_CREATIONTIME(nick, channel_name, creation_time)		std::string(":") + SERVER_NAME + " 329 " + nick + " " + channel_name + " " + creation_time
#define	RPL_NOTOPIC(nick, channel_name)							std::string(":") + SERVER_NAME + " 331 " + nick + " " + channel_name + " :No topic is set"
#define	RPL_TOPIC(nick, channel_name, topic)					std::string(":") + SERVER_NAME + " 332 " + nick + " " + channel_name + " :" + topic
#define RPL_WHOREPLY(nick, usernmame)							std::string(":") + SERVER_NAME + " 352 " + nick + " * ~" + nick + " localhost " + SERVER_NAME + " " + nick + " H :0 " + usernmame
#define RPL_BANLIST(nick, channel_name, user_nick)				std::string(":") + SERVER_NAME + " 367 " + nick + " " + channel_name + " " + user_nick
#define RPL_ENDOFBANLIST(nick, channel_name)					std::string(":") + SERVER_NAME + " 368 " + nick + " " + channel_name + " :End of channel ban list"

#define ERROR(text)												std::string(":") + "ERROR :" + text
#define HELP(nick, to, message)									std::string(":") + SERVER_NAME + " NOTICE " + nick + " :" + message
#define JOIN(nick, username, channel_name)						std::string(":") + nick + " JOIN :" + channel_name
#define KICK(nick, username, channel_name, target, reason)		std::string(":") + username + "!~" + nick + " KICK " + channel_name + " " + target + " :" + reason
#define NICK(old_nick, username, new_nick)						std::string(":") + username + "!~" + old_nick + " NICK :" + new_nick
#define MODE_BAN(nick, channel_name, user_nick)					std::string(":") + nick + " MODE " + channel_name + " +b " + user_nick
#define MODE_UNBAN(nick, channel_name, user_nick)				std::string(":") + nick + " MODE " + channel_name + " -b " + user_nick
#define MODE_OP(nick, channel_name, user_nick)					std::string(":") + nick + " MODE " + channel_name + " +o " + user_nick
#define MODE_DEOP(nick, channel_name, user_nick)				std::string(":") + nick + " MODE " + channel_name + " -o " + user_nick
#define MODE_TOPICPROTECTED(nick, channel_name)					std::string(":") + nick + " MODE " + channel_name + " +t"
#define MODE_TOPICUNPROTECTED(nick, channel_name)				std::string(":") + nick + " MODE " + channel_name + " -t"
#define NOTICE(nick, username, to, message)						std::string(":") + nick + "!~" + username + " NOTICE " + to + " :" + message
#define PART(nick, username, channel_name, reason)				std::string(":") + nick + "!~" + username + " PART " + channel_name + " :" + reason
#define PRIVMSG(nick, username, to, text)						std::string(":") + nick + "!~" + username + " PRIVMSG " + to + " :" + text
#define TOPIC(nick, username, channel_name, topic)				std::string(":") + nick + " TOPIC " + channel_name + " :" + topic
