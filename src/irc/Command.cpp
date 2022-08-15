#include "irc/Command.hpp"
#include "irc/Server.hpp"

namespace ft
{

Command::Command(Server& server, const std::string& name, const std::string& description)
	: _name(name), _description(description), _server(server)
{	}

Command::~Command()
{	}

const char *Command::parse_arguments(const char *cmd, std::vector<std::string>& args)
{
	cmd = this->parse_arguments_space(cmd, args);
	if (!*cmd) {
		return cmd;
	}
	return this->parse_arguments_colon(cmd, args);
}

const char *Command::parse_arguments_space(const char *cmd, std::vector<std::string>& args)
{
	for (;;) {
		std::string arg;
		while (*cmd && *cmd != ' ') {
			arg += *cmd;
			++cmd;
		}
		if (!arg.empty()) {
			args.push_back(arg);
		}
		while (*cmd == ' ') {
			++cmd;
		}
		if (!*cmd || *cmd == ':') {
			break ;
		}
	}
	return cmd;
}

const char *Command::parse_arguments_colon(const char *cmd, std::vector<std::string>& args)
{
	if (*cmd == ':')
		++cmd;

	for (;;) {
		std::string arg;
		while (*cmd && *cmd != ':') {
			arg += *cmd;
			++cmd;
		}

		/* Trim end spaces */
		if (!arg.empty())
		{
			while (arg[arg.size() - 1] == ' ')
			{
				arg.pop_back();
			}
		}

		args.push_back(arg);

		if (*cmd == ':') {
			++cmd;
		}
		if (!*cmd) {
			break ;
		}
	}
	return cmd;
}

}
