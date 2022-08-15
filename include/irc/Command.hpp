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

}

}
