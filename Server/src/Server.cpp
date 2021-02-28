#include "Server.h"
#include "ErrorMessages.h"


Server::Server(int port, int maxClients)
{	
	this->port = port;
	this->max_clients = maxClients;
	current_num_of_clients = 0;

}

void Server::createFileDescriptor()
{
	std::cout << "Creating a socket file descriptor" << std::endl;
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror(CREATING_FILE_DESCRIPTOR_ERROR.c_str());
		exit(EXIT_FAILURE);
	}
}

void Server::attachSocket()
{
	std::cout << "Attaching socket to the port" << std::endl;
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror(ATTACHING_SOCKET_DESCRIPTOR_ERROR.c_str());
		exit(EXIT_FAILURE);
	}
}

void Server::bindServer()
{
	std::cout << "Binding the server" << std::endl;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	signal(SIGPIPE, SIG_IGN);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		perror(BINDING_SERVER_DESCRIPTOR_ERROR.c_str());
		exit(EXIT_FAILURE);
	}
}

void Server::listenForConnections()
{
	std::cout << "<i> SERVER STARTED ON PORT: " << this->port << ", MAXIMUM CLIENTS: " << this->max_clients << std::endl;
	if (listen(server_fd, 5) < 0)
	{
		perror(LISTENING_FOR_CONNECTIONS_ERROR.c_str());
		exit(EXIT_FAILURE);
	}
	std::cout << "Listening for incoming connections... " << std::endl;
}

void Server::startServer() {
    createFileDescriptor();
    attachSocket();
    bindServer();
    listenForConnections();
    run();
}

void Server::run()
{
	int socket;
	int addrlen = sizeof(address);

	while (1)
	{
		if ((socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0)
		{
			perror(ACCEPTING_CONNECTION_ERROR.c_str());
			exit(EXIT_FAILURE);
		}
		std::string clientIp = ipToString(address);
		std::cout << "New Client with ip: "+clientIp+" just connected to the server" << std::endl;

		if (current_num_of_clients == max_clients)
		{
			std::cout << "Maximum number of clients reached" << std::endl;
			std::cout << "Client has been disconnected!" << std::endl;
			close(socket);
			continue;
		}

		current_num_of_clients++;
		Client* client = new Client(socket, clientIp);
		std::thread clientHandler(&Server::handleClient, this, client);
		clientHandler.detach();
		usleep(100000);
	}
}

/// Thread ensuring that the client is still connected/communicating with the server
void Server::handleClientPing(Client *client)
{
	int time = 0;
	//int remainingTime;
	std::string clientStringRepresentation = "UNKNOWN";
	Client::State state;

	while(time != REPLY_TIMELIMIT)
	{
		client_mutex.lock();
		state = client->getState();
		clientStringRepresentation = client->toString();
		client_mutex.unlock();

		if (state == Client::kEnd) {
			return;
		}

		//remainingTime = 8 - time;
		//std::cout << "Waiting for client: " + clientStringRepresentation + " for " + std::to_string(remainingTime) + " s" << std::endl;
		client_mutex.lock();
		if (client->getPinged())
		{
			client->setPinged(false);
			time = 0;
		}
		else time++;
		client_mutex.unlock();
		sleep(1);
	}

	std::cout << "Client: " + clientStringRepresentation + " has not sent a Ping withing " + std::to_string(REPLY_TIMELIMIT) + "s" << std::endl;
	client_mutex.lock();
	client->setConnected(false);
	client_mutex.unlock();
}

void Server::handleClient(Client* client)
{
	std::cout << "Handling client " + client->toString() << std::endl;
	char buffer[BUFFER_SIZE];
	int message_len;
	std::string recieved_message;
	std::vector<std::string> tokens;
	incomingMsgHeaders header;
	std::thread clientPingThread;
	std::thread clientReconnectingThread;

	std::string receiver;
    std::string sender;


	fd_set sockets;
	FD_ZERO(&sockets);
	struct timeval timeout;

	clientPingThread = std::thread(&Server::handleClientPing, this, client);
    clientPingThread.detach();
	
	while (client->isConnected())
	{
		FD_SET(client->getSocket(), &sockets);
		timeout.tv_sec  = 0;
        timeout.tv_usec = SOCKET_TIMELIMIT;
        select(FD_SETSIZE, &sockets, NULL, NULL, &timeout);

		if (FD_ISSET(client->getSocket(), &sockets))
		{
			if ((message_len = recv(client->getSocket(), buffer, sizeof(buffer) - 1, 0)) > 0)
			{
				buffer[message_len] = '\0';
				stripNewLineChar(buffer);
				if((recieved_message = std::string(buffer))== "")
				{
					continue;
				}

				// if First char isn't coming from Client -> prefix isn't 'C' = invalid message 
				if (recieved_message.at(0) != 'C')
					header = kUnknown;
				else
				{
					tokens = tokenize(recieved_message);
					header = getMessageType(tokens[0]);
				}

				//std::cout << "Recieved msg from client:" + recieved_message  << std::endl;
					
					
				if(header == kUnknown)
				{
					std::cout << "Recieved unknown msg type from client"  << std::endl;

					if (client->getState() == Client::kPlaying)
						deleteGameRoom(client->getUsername());
					if (client->getState() == Client::kJoining)
					{
						Client::State state = client->getState();
						client->setState(Client::kEnd);
                        sleep(2);
                        client->setState(state);
					}

					client->setConnected(false);
					deleteClient(client);
					return;
				}
				else if (header == kCexit)
				{
					broadcastToOtherClients(client->getUsername(), "SDELP~" + client->getUsername());
					if (client->getState() == Client::kPlaying)
						deleteGameRoom(client->getUsername());
					else if (client->getState() == Client::kSentRq)
					{
						broadcastToOtherClients(receiver, "SUPS~"+receiver+"READY");	
					}
					else if (client->getState() == Client::kRecvRq)
					{
						//TODO: getGameRequestSender
						broadcastToOtherClients(sender, "SUPS~"+sender+"READY");
					}
					if (client->getState() == Client::kSentRq || client->getState() == Client::kRecvRq)
					{
						//TODO: deleteGameRequest
					}
						
					Client::State state = client->getState();
					client->setState(Client::kEnd);
					sleep(2);
					client->setState(state);

					deleteClient(client);
					return;

				}
				else if (header == kCping)
				{
					client->send("SPONG");
					client->setPinged(true);
				}
				else {
					switch (client->getState())
					{
						case Client::kJoining:
							if (header != kCname)
							{
								std::cout << client->toString() + " is not following the protocol and will be disconnected" << std::endl;
								deleteClient(client);
								return;
							}
							if (isUsernameInUse(tokens[1]))
							{
								std::cout << client->toString() + " is trying to set their nickname to one that is already in use" << std::endl;
								client->setState(Client::kEnd);
								sleep(2);
								deleteClientWithReference(client);
								return;
							}
							client->setUsername(tokens[1]);
							addClient(client);
							client->setState(Client::kLobby);
							std::cout << client->toString() << std::endl;
							updateAllOnlineClients(client);
							updateAllBusyClients(client);

							if(isReconnecting(client->getUsername()))
							{
								removePlayerFromReconnectingList(client->getUsername(), true);
							}

							break;

						case Client::kLobby:
							if (header != kCreq)
							{
								std::cout << client->toString() + " is not following the protocol and will be disconnected" << std::endl;
								deleteClient(client);
								return;
							}
							else if (isUsernameInUse(tokens[1]) == false)
							{
								std::cout << client->toString() + " is trying to send a game request to a non existent user" << std::endl;
								deleteClient(client);
								return;
							}
							else if (tokens[1] == client->getUsername())
							{
								std::cout << client->toString() + " is trying to send a game request to himself" << std::endl;
								deleteClient(client);
								return;
							}
							else if (getClientState(tokens[1]) != Client::kLobby)
							{
								std::cout << client->toString() + " is trying to send a game request to client that is not in the lobby" << std::endl;
								deleteClient(client);
								return;
							}

							receiver = tokens[1];
							client->setState(Client::kSentRq);
							setClientState(tokens[1], Client::kRecvRq);

							sendMessage(tokens[1], "SREQ~"+client->getUsername());

							broadcastToOtherClients(client->getUsername(), "SUPS~"+client->getUsername()+"~BUSY");
							broadcastToOtherClients(receiver, "SUPS~"+receiver+"~BUSY");

							//TODO: AddGameRequest
							//TODO: ?WaitingForResponseHandler?
							break;

						case Client::kSentRq:
							if (header != kCreqc)
							{
								std::cout << client->toString() + " is not following the protocol and will be disconnected" << std::endl;
								deleteClient(client);
								return;
							}
							else if (isUsernameInUse(tokens[1]) == false)
							{
								std::cout << client->toString() + " is trying to cancel game request of non existent user" << std::endl;
								deleteClient(client);
								return;
							}
							else if (receiver != tokens[1])
							{
								std::cout << client->toString() + " is trying to cancel game request that he did not created" << std::endl;
								deleteClient(client);
								return;
							}
							sendMessage(tokens[1], "SREQC~"+client->getUsername());
                            client->setState(Client::kLobby);
							setClientState(tokens[1], Client::kLobby);

							broadcastToOtherClients(client->getUsername(), "SUPS~"+client->getUsername()+"~READY");
							broadcastToOtherClients(receiver, "SUPS~"+receiver+"~READY");
							break;

						case Client::kRecvRq:
							if (header != kCres)
							{
								std::cout << client->toString() + " is not following the protocol and will be disconnected" << std::endl;
								deleteClient(client);
								return;
							}
							else if (isUsernameInUse(tokens[1]) == false)
							{
								std::cout << client->toString() + " is trying to reply to game request of non existent user" << std::endl;
								deleteClient(client);
								return;
							}
							// TODO: Sender != tokens[1]
							if(tokens[2] == "ACCEPT")
							{
								client->setState(Client::kPlaying);
								setClientState(tokens[1], Client::kPlaying);

								client->send("SSTARTG~"+tokens[1]);
								sendMessage(tokens[1], "SSTARTG~"+client->getUsername());

								addGameRoom(tokens[1], client->getUsername());
								std::cout << "A game of Othello: '" +tokens[1]+ "' vs '" + client->getUsername() + "' just started!"  << std::endl;


							}
							else if (tokens[2] == "DECLINE")
							{
								sendMessage(tokens[1], "SREQC~" + client->getUsername());
								client->setState(Client::kLobby);
								setClientState(tokens[1], Client::kLobby);
								
								// TODO: UPDATE player states
								broadcastToOtherClients(client->getUsername(), "SUPS~"+client->getUsername()+"~READY");
								broadcastToOtherClients(tokens[1], "SUPS~"+tokens[1]+"~READY");

								std::cout << "Client '" + client->getUsername() + "' rejeceted a game request from client '"+ tokens[1] +"'"  << std::endl;
							}
							break;

						case Client::kPlaying:
							if (header == kCmov)
							{
								int xPosition = stoi(tokens[1]);
								int yPosition = stoi(tokens[2]);
								game_room_mutex.lock();
								Othello *game = gameRooms[client->getUsername()]->game;
								Othello::GameState gameState = game->makeMove(xPosition, yPosition);
								game_room_mutex.unlock();
								//game->printBoard();

								if(gameState != Othello::kInProgress) {
									deleteGameRoom(client->getUsername());
									client->send("The Game is over");
								}
							}
							else if (header == kCgamec)
							{
								deleteGameRoom(client->getUsername());
								client->send("SGAMEC~You canceled the game");
							}
							else
							{
								std::cout << client->toString() + " is not following the protocol and will be disconnected" << std::endl;
								deleteGameRoom(client->getUsername());
								deleteClient(client);
								return;
							}
							break;

						case Client::kEnd:
							break;
					}
				}
			}
			
		}	
	}

	std::cout << "Lost connectin with the client " << std::endl;
	if (client->getState() == Client::kSentRq)
	{
		broadcastToOtherClients(receiver, "SUPS~" + receiver + "~READY");
	}

	if (client->getState() == Client::kPlaying)
	{
		bool opponentInGame = isOpponentInGame(client->getUsername());
		std::string opponent = getOpponent(client->getUsername(), true);
		removePlayerFromGameRoom(client->getUsername());

		if(opponentInGame) {
			std::cout << client->getUsername() + " lost their connecting, waiting for them to reconnect" << std::endl;
			addPlayerToReconnectingList(client->getUsername(), opponent);
			sendMessage(opponent, "SGAMEI~Your opponent has connection problems and has 60 sec to reconnect, otherwise the game will end");
			clientReconnectingThread = std::thread(&Server::waitForPlayerToReconnect, this, client->getUsername(), opponent);
			clientReconnectingThread.detach();
		}
		else removeBothPlayersFromReconnectingList(client->getUsername(), opponent);
	}

	Client::State state = client->getState();
    client->setState(Client::kEnd);
	sleep(2);
	client->setState(state);
	deleteClient(client);
	
}


void Server::broadcastToOtherClients(std::string sendingClientName, std::string data)
{
	for (auto it : clients)
	{
		if (it.first != sendingClientName)
		{
			it.second->send(data);
		}
			
	}

}

void Server::updateAllOnlineClients(Client *client)
{
	client_mutex.lock();
	for (auto it : clients)
	{
		if (it.first != client->getUsername())
			client->send("SADDP~" + it.first);
	}
	client_mutex.unlock();
}

void Server::updateAllBusyClients(Client *client)
{
	client_mutex.lock();
	for (auto it : clients)
	{
		if (it.first != client->getUsername()){
			Client::State state = it.second->getState();
            if (state == Client::kPlaying || state == Client::kRecvRq || state == Client::kSentRq)
                client->send("SUPS~" + it.first + "~BUSY");
		}
	}
	client_mutex.unlock();
}

std::string Server::ipToString(struct sockaddr_in address) const {
	unsigned long addr = address.sin_addr.s_addr;
	char buffer[50];
	sprintf(buffer, "%d.%d.%d.%d",
		(int)(addr & 0xff),
		(int)((addr & 0xff00) >> 8),
		(int)((addr & 0xff0000) >> 16),
		(int)((addr & 0xff000000) >> 24));
	return std::string(buffer);
}

void Server::sendMessage(std::string toWho, std::string data)
{
	client_mutex.lock();
	auto it = clients.find(toWho);
	if (it != clients.end())
		it->second->send(data);
	else
	{
		std::cout << "Trying to sending message to a client ('" + toWho + "') that doesn't exist!" << std::endl;
	}
	client_mutex.unlock();
	
}

void Server::addClient(Client *client)
{
	client_mutex.lock();
	clients[client->getUsername()] = client;
	broadcastToOtherClients(client->getUsername(), "SADDP~" + client->getUsername());
	client_mutex.unlock();
}

void Server::deleteClientWithReference(Client* client)
{
	std::cout << "Closing connection for the client " + client->toString() << std::endl;
	delete client;
}

void Server::deleteClientWithName(std::string username)
{
	client_mutex.lock();
	deleteClientWithReference(clients[username]);
	clients.erase(username);
	broadcastToOtherClients(username, "SDELP~"+ username);
	client_mutex.unlock();

}

void Server::deleteClient(Client *client)
{

	current_num_of_clients--;

	if(client->getState() == Client::kJoining)
	{
		deleteClientWithReference(client);
	}
	else {
		deleteClientWithName(client->getUsername());
	}
}


void Server::setClientState(std::string clientName, Client::State state)
{
	client_mutex.lock();
	auto it = clients.find(clientName);
	if (it != clients.end())
	{
		it->second->setState(state);
	}
	else
	{
		std::cout << "Could not change clients: '"+ clientName +"' state. It is possible that client does no longer exists" << std::endl;
	}
	client_mutex.unlock();
}



void Server::addGameRoom(std::string player1, std::string player2)
{
	game_room_mutex.lock();

	Othello *game = new Othello(player1, player2, this);
	GameRoom *gameRoom = new GameRoom{player1, player2, game};
	gameRooms[player1] = gameRoom;
	gameRooms[player2] = gameRoom;

	game_room_mutex.unlock();

}

void Server::deleteGameRoom(std::string player)
{
	game_room_mutex.lock();
	std::string opponent = getOpponent(player, false);

	if (gameRooms.find(opponent) != gameRooms.end())
	{
        gameRooms.erase(opponent);
        setClientState(opponent, Client::kLobby);
        sendMessage(opponent, "SGAMEC~GameHasBeenCanceled");
        broadcastToOtherClients(opponent, "SUPS~"  + opponent + "~READY");
    }
	else
	{
		reconnect_mutex.lock();
		reconnectingList.erase(opponent);
		reconnect_mutex.unlock();
	}

	if (gameRooms.find(player) == gameRooms.end())
	{
		game_room_mutex.unlock();
		return;
	}

	delete gameRooms[player]->game;
	delete gameRooms[player];
	gameRooms.erase(player);
	setClientState(player, Client::kLobby);
	broadcastToOtherClients(player, "SUPS~"  + player + "~READY");

	game_room_mutex.unlock();
} 

void Server::stripNewLineChar(char* data) const
{
	while(*data != '\0')
	{
		if (*data == '\r' || *data == '\n')
		{
			*data = '\0';
		}
		data++;
	}
}

bool Server::isUsernameInUse(std::string clientName)
{
	client_mutex.lock();
	bool found = clients.find(clientName) != clients.end();
	client_mutex.unlock();
	return found;
}

void Server::addPlayerToReconnectingList(std::string player, std::string opponent)
{
	reconnect_mutex.lock();
	reconnectingList[player] = opponent;
	reconnect_mutex.unlock();
}

void Server::addPlayersToGameRoom(std::string player, std::string opponent)
{
	game_room_mutex.lock();
	gameRooms[player] = gameRooms[opponent];
	setClientState(player, Client::kPlaying);
	sendMessage(player, "SSTARTG~"+opponent);
	//TODO: Send current game state on next line
	sendMessage(player, "SRECOV~"+gameRooms[player]->game->getGameState(player));
	//Watching thread??
	game_room_mutex.unlock();
}

void Server::removePlayerFromGameRoom(std::string player)
{
	game_room_mutex.lock();
    std::string opponent = getOpponent(player, false);
    if (gameRooms.find(opponent) == gameRooms.end()) {
        removeBothPlayersFromReconnectingList(player, opponent);
        delete gameRooms[player]->game;
        delete gameRooms[player];
    }
    else {
        //sendMessage(opponent, O_GAME_MESSAGE + " other player lost their connection. Waiting for him " + std::to_string(SECONDS_WAITING_FOR_DISCONNECTED_PLAYER) + "s");
		//watchingThread??
    }
    gameRooms.erase(player);
    game_room_mutex.unlock();
}

void Server::removePlayerFromReconnectingList(std::string player, bool reconnected)
{
	reconnect_mutex.lock();
	std::string opponent = reconnectingList[player];
	if (reconnected)
	{
		addPlayersToGameRoom(player, opponent);
		broadcastToOtherClients(player, "SUPS~"+ player +"~BUSY");
	}
	else
	{
		sendMessage(opponent, "SGAMEC~GameHasBeenCanceled");
		deleteGameRoom(opponent);
		setClientState(opponent, Client::kLobby);
	}
	reconnectingList.erase(player);
	reconnect_mutex.unlock();
}

void Server::removeBothPlayersFromReconnectingList(std::string player1, std::string player2)
{
	reconnect_mutex.lock();
	if (reconnectingList.find(player1) != reconnectingList.end())
		reconnectingList.erase(player1);
	if (reconnectingList.find(player2) != reconnectingList.end())
		reconnectingList.erase(player2);
	reconnect_mutex.unlock();
}

void Server::waitForPlayerToReconnect(std::string player, std::string opponent)
{
	for (int i = 0; i < RECONNECT_TIMELIMIT; i++)
	{
		if (!isReconnecting(player) || !isInTheGame(opponent))
		{
			std::cout << "Waiting for client " + player + " to reconnect was interrupted" << std::endl;
			removeBothPlayersFromReconnectingList(player, opponent);
			sendMessage(opponent, "SGAMEI~Your opponent just reconnected!");
			return;
		}
		int remainingTime = RECONNECT_TIMELIMIT - i;
		std::cout << "Waiting for client " + player + " to reconnect back to the server for " + std::to_string(remainingTime) + "s" << std::endl;
		sleep(1);
	}
	removePlayerFromReconnectingList(player, false);
}


Server::incomingMsgHeaders Server::getMessageType(std::string header)
{
	if (header == "CPING") return Server::kCping; //ping server
	else if(header == "CNAME") return Server::kCname; //set name
	else if (header == "CMOV") return Server::kCmov; //game move
	else if (header == "CREQ") return Server::kCreq; //request
	else if (header == "CRES") return Server::kCres; //response
	else if (header == "CREQC") return Server::kCreqc; //cancel game request
	else if (header == "CGAMEC") return Server::kCgamec; //cancel the game
	else if (header == "CEXIT") return Server::kCexit; //exit game
	else return Server::kUnknown; //unknows message
	
}

std::vector<std::string> Server::tokenize(const std::string& data)
{
	std::stringstream ss(data); 
	std::string token;
	std::vector<std::string> tokens;

	while(std::getline(ss, token, SEPARATOR))
	{
		if (token != "")
		tokens.emplace_back(token); 
	}

	return tokens;
}

Client::State Server::getClientState(std::string clientName)
{
	client_mutex.lock();
	Client::State state = clients[clientName]->getState();
	client_mutex.unlock();
	return state;
}


bool Server::isInTheGame(std::string player)
{
	game_room_mutex.lock();
	bool stillInGame = gameRooms.find(player) != gameRooms.end();
	game_room_mutex.unlock();
	return stillInGame;
}

bool Server::isOpponentInGame(std::string player)
{
	game_room_mutex.lock();
	std::string opponent = getOpponent(player, false);
	bool stillInGame = gameRooms.find(opponent) != gameRooms.end();
	game_room_mutex.unlock();
	return stillInGame;
}

bool Server::isReconnecting(std::string player)
{
	reconnect_mutex.lock();
	bool found = reconnectingList.find(player) != reconnectingList.end();
	reconnect_mutex.unlock();
	return found;
}

std::string Server::getOpponent(std::string playerName, bool lock)
{
	if (lock)
		game_room_mutex.lock();
	if (gameRooms.find(playerName) == gameRooms.end())
	{
		if (lock)
			game_room_mutex.unlock();
		return Client::UNKNOWN;
	}
	std::string opponent = Client::UNKNOWN;

	if (gameRooms[playerName]->player1 == playerName)
		opponent = gameRooms[playerName]->player2;
	else 
		opponent = gameRooms[playerName]->player1;
	
	if (lock)
		game_room_mutex.unlock();
	
	return opponent;
}