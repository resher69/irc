#include <iostream>
#include "irc/Server.hpp"
#include "Colors.hpp"

#include <string>
#include <vector>

bool is_all_numbers(const std::string& str)
{
	for (size_t i = 0; i < str.size(); ++i) {
		if (str[i] < '0' || str[i] > '9') {
			return false;
		}
	}
	return true;
}

int main(int argc, char **argv)
{
	ft::Server *server;

	std::vector<std::string> args;

	++argv;
	while (--argc) {
		args.push_back(*argv++);
	}

	if (args.size() == 0 || args.size() > 2) {
		std::cerr << PRINT_FATAL << "Usage: ./ircserv <port> [<password>]" << std::endl;
		return EXIT_FAILURE;
	}

	if (!is_all_numbers(args[0])) {
		std::cerr << PRINT_FATAL << "Port should be a number. '" << args[0] << "' is invalid" << std::endl;
		return EXIT_FAILURE;
	}

	std::string password;
	if (args.size() == 2) {
		password = args[1];
	}

	try {
		server = new ft::Server(::atoi(&args[0][0]), password);
	} catch (const std::exception& e) {
		std::cerr << PRINT_FATAL << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	for (;;)
	{
		try {
			server->update();
		} catch (const std::exception &e) {
			std::cerr << PRINT_FATAL << e.what() << std::endl;
			return EXIT_FAILURE;
		}
	}

	delete server;
}
