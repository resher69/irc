#include <iostream>
#include "irc/Server.hpp"
#include "Colors.hpp"

int main()
{
	ft::Server *server;

	try {
		server = new ft::Server(6667, "abc");
	} catch (const std::exception& e) {
		std::cerr << FATAL << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	for (;;)
	{
		try {
			server->update();
		} catch (const std::exception &e) {
			std::cerr << FATAL << e.what() << std::endl;
			return EXIT_FAILURE;
		}
	}

	delete server;
}
