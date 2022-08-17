#pragma once

#include <string>
#include <vector>
#include "irc/Client.hpp"
#include "irc/Server.hpp"

namespace ft
{

class Command
{
	protected:
		std::string _name;
		std::string _description;
		Server& _server;

	protected:
		Command(Server& server, const std::string& name, const std::string& description);
		const char *parse_arguments(const char *cmd, std::vector<std::string>& args);

	public:
		virtual void execute(Client *sender, const std::string& args) = 0;
		virtual ~Command();

		const std::string& name() const;
		const std::string& description() const;

	private:
		const char *parse_arguments_space(const char *cmd, std::vector<std::string>& args);
		const char *parse_arguments_colon(const char *cmd, std::vector<std::string>& args);

};

namespace cmd
{

class Quit : public Command
{
	public:
		Quit(Server& server);
		void execute(Client *sender, const std::string& args);
};

class Pass : public Command
{
	public:
		Pass(Server& server);
		void execute(Client *sender, const std::string& args);
};

class Nick : public Command
{
	public:
		Nick(Server& server);
		void execute(Client *sender, const std::string& args);
};

class User : public Command
{
	public:
		User(Server& server);
		void execute(Client *sender, const std::string& args);
};

class Ping : public Command
{
	public:
		Ping(Server& server);
		void execute(Client *sender, const std::string& args);
};

class Join : public Command
{
	public:
		Join(Server& server);
		void execute(Client *sender, const std::string& args);

	private:
		struct ChannelInfo {
			std::string name;
			std::string key;
		};

	private:
		void parse_infos	(	const std::string& channel_string,
								const std::string& key_string,
								std::vector<ChannelInfo>& channelinfos	);

};

class Privmsg : public Command
{
	public:
		Privmsg(Server& server);
		void execute(Client *sender, const std::string& args);
};

class Help : public Command
{
	public:
		Help(Server& server);
		void execute(Client *sender, const std::string& args);
};

class List : public Command
{
	public:
		List(Server& server);
		void execute(Client *sender, const std::string& args);
};

class Notice : public Command
{
	public:
		Notice(Server& server);
		void execute(Client *sender, const std::string& args);
};

class Topic : public Command
{
	public:
		Topic(Server& server);
		void execute(Client *sender, const std::string& args);
};

class Kick : public Command
{
	public:
		Kick(Server& server);
		void execute(Client *sender, const std::string& args);
};

class Part : public Command
{
	public:
		Part(Server& server);
		void execute(Client *sender, const std::string& args);
};

}

}
