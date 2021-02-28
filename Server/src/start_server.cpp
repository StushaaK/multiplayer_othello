#include "Server.h"


void printBasicInfo()
{
	std::cout << "===>---------- SERVER SETTINGS ----------<===" << std::endl;
	std::cout << "Please complete the basic setting of the server..." << std::endl;
	std::cout << "Leave the fields empty and press <ENTER> for the default values" << std::endl;
	std::cout << "The default values are: " << std::endl;
	std::cout << "PORT: " << 6321 << std::endl;
	std::cout << "MAXIMUM NUMBER OF CLIENTS: " << 12 << std::endl;
	std::cout << std::endl;

}

void printHelp()
{
	std::cout << "You entered invalid values..." << std::endl;
	std::cout << "Valid values for the port are 0 - 65535" << std::endl;
	std::cout << "Valid values for the number of clients are 2 - 120" << std::endl;
	std::cout << "Server was terminated!" << std::endl;
}

void getInitValues(int& port, int& max_players)
{
	std::cout << "Type the PORT on which the server will be running: ";
    
	if (std::cin.peek() == '\n')
	{
		port = Server::DEFAULT_PORT;
	}
	else if (!(std::cin >> port))
	{
		printHelp();
		exit(EXIT_FAILURE);
	}
    
	if (port < 0 || port > 65535)
	{
		printHelp();
		exit(EXIT_FAILURE);
	}

	std::cin.clear();
    std::cin.ignore();

	std::cout << "Type the MAXIMUM NUMBER OF CLIENTS that can be connected to the server: ";

	if (std::cin.peek() == '\n')
	{
		max_players = Server::DEFAULT_MAX_CLIENTS;
	}
	else if (!(std::cin >> max_players))
	{
		printHelp();
		exit(EXIT_FAILURE);
	}
    
	if (max_players < 2 || max_players > 50)
	{
		printHelp();
		exit(EXIT_FAILURE);

	}

	std::cout << std::endl;
}

int main(int argc, char* argv[])
{
	int port = 0;
	int max_players = 0;

	printBasicInfo();
	getInitValues(port, max_players);
	Server server(port, max_players);

	std::cout << "===>---------- STARTING SERVER ----------<===" << std::endl;

	server.startServer();
	return 0;
}

