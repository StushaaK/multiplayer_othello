#pragma once

#include <algorithm>
#include <string>
#include <map>
#include <unordered_map>

#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <bits/stdc++.h> 

#include "Client.h"
#include "ErrorMessages.h"
#include "Othello.h"


/// forward declaration
class Client;
class Othello;

/// \author Daniel Stuš
///
/// Class representing the server itself.
/// Handles the connection of the clients and listen for their messages.
/// It generates new thread for each client
class Server
{
public:

	// --- GLOBAL SERVER SETTINGS ---
	/// default port o which the server runs
	static const int DEFAULT_PORT = 6321;

	/// default number of clients that can be connected to the serer at once
	static const int DEFAULT_MAX_CLIENTS = 12;

	// STREAM TEXT HANDLING - SEPARATOR & BUFFER
	/// message separator in the protocol
	static const char SEPARATOR = '~';

	/// size of the buffer for the messages
	static const int BUFFER_SIZE = 256;

	// TIMELIMITS IN SECONDS

	/// timilimit for the user to reconnect
	static const int RECONNECT_TIMELIMIT = 60;

	/// timeout in microseconds for the select method
	static const int SOCKET_TIMELIMIT = 10000;


	/// timelimit for the reply to the ping message
	static const int REPLY_TIMELIMIT = 10;

	/// types of incoming messages from client
	enum incomingMsgHeaders
	{
		kCping,
		kCname,
		kCreq,
		kCres,
		kCreqc,
		kCmov,
		kCgamec,
		kCexit,
		kUnknown
	};

private:

	/// strucutre holding information about gameRoom
	/// the game itself and two players playing it
	struct GameRoom
	{
		std::string player1;
		std::string player2;
		Othello *game;
	};

	/// socket filedescriptor
	int server_fd, new_socket, valread;

	/// socket address
	struct sockaddr_in address;

	/// port on which the server runs
	int port;

	/// maximum number of clients that can be connected to the server at one time
	int max_clients;

	/// current number of clients on the server
	int current_num_of_clients;

	/// mutex used for accessing clients
	std::mutex client_mutex;

	/// mutex used for acessing game rooms
	std::mutex game_room_mutex;

	/// mutex used for accessing reconnecting list
	std::mutex reconnect_mutex;

	/// map holding all clients connected to the server
	/// key - username of the client, value - reference to client
	std::unordered_map<std::string, Client *> clients;

	/// map holding information about which player is in which gameRoom
	/// key - username of the client, value - reference of GameRoom
	std::unordered_map<std::string, GameRoom *> gameRooms;

	/// map holding information about clients that lost their connection and can reconnect
	/// key - username of the client, value - username of the opponent
	std::unordered_map<std::string, std::string> reconnectingList;

public:
	// ----- BASIC PUBLIC SERVER FUNCTIONALITY -----

	/// Constructor of the Server
	///
	/// \param port - the port number that the server is running on
	/// \param maxClients - maximum number of clients that can be connected to the server at one time
	Server(int port, int maxClients);

	/// Copy constructor of the class. It has no use in the scope of this project, so it was deleted.
	Server(Server&) = delete;

	/// Starts the server
	void startServer();

	/// Assignment operator of the class. It has no use in the scope of this project, so it was deleted.
	void operator = (Server const&) = delete;

	/// Sens a message to the client given in paramameters nick and msg
	///
	/// \param username - username of the client to whom the message will be sent
	/// \param msg - message to be sent
	void sendMessage(std::string username, std::string msg);

	/// Creates a bew gane riin between two players given as parameters
	///
	/// \param player1 - username of the first player
	/// \param player2 - username of the second player
	void addGameRoom(std::string player1, std::string player2);

	/// Deletes a game room of the player given as parameter
	///
	/// \param player - nick of the player whos game should be deleted
	void deleteGameRoom(std::string player);

private:

	// ----- BASIC PRIVATE SERVER FUNCTIONALITY -----

	/// Creates a new file descriptor for the server
	void createFileDescriptor();

	/// Attaches the socket to the port
	void attachSocket();

	/// Bind the server to the socket
	void bindServer();

	/// Start listening for the incomming connections
	void listenForConnections();

	/// Starts the main thread accepting connection from the clients
	void run();
	
	// ----- CLIENT -----
	/// Thread that handles client that is given as parameter
	///
	/// \param client - reference to the client that the thread will be handling
	void handleClient(Client* client);

	/// Thread that handles ping messages between the client and server
	///
	/// \param client - reference to the client that is sending the ping messages
	void handleClientPing(Client *client);

	/// Adds the new client given as a parameter to the map of all clients
	///
	/// \param client - reference to the client that should be added
	void addClient(Client* client);

	/// Deletes the client from map of all clients and memory
	///
	/// \param client - reference to the client that should be deleted
	void deleteClient(Client* client);

	/// Delete client given as a parameter from memory
	///
	/// \param client - reference to the client that should be deleted
	void deleteClientWithReference(Client* client);

	/// Delete the client from map of all clients
	///
	/// \param username - username of the client that should be deleted
	void deleteClientWithName(std::string username);

	/// Returns the current state of the client
	///
	/// \param clientName - username of the client
	/// \return Current state of the client
	Client::State getClientState(std::string clientName);

	/// Sets the client state to the new value
	///
	/// \param clientName - username of the client
	/// \param state - new state of the client
	void setClientState(std::string clientName, Client::State state);

	/// Checks if the username is already in use
	///
	/// \param clientName - username of the client
	/// \return true - if the username is already in use (client with that name is already connected to the server), false otherwise
	bool isUsernameInUse(std::string clientName);

	/// Adds the player to the list of all players waiting for reconnecting
	///
	/// \param player - player that lost connection
	/// \param oponnent - opponent who is still in the game
	void addPlayerToReconnectingList(std::string player, std::string opponent);

	/// Removes player from reconnection list
	///
	/// \param player - name of the player that should be removed from reconnecting list
	/// \param reconnected - true, if the player successfully reconnected, false otherwise
	void removePlayerFromReconnectingList(std::string player, bool reconnected);

	/// Removes both players given as parameters from the reconnecting list
	///
	/// \param player1 - name of the first player that should be removed from reconnecting list
	/// \param player2 - name of the second player that should be removed from reconnecting list
	void removeBothPlayersFromReconnectingList(std::string player1, std::string player2);

	/// Thread that handles the waiting for player to reconnect back to the game
	///
	/// \param player1 - name of the player that lost ocnnection
	/// \param opponent - name of the opponent that is still waiting in the game
	void waitForPlayerToReconnect(std::string player, std::string opponent);

	/// Adds the client that just connected back to the game room
	///
	/// \param player - name of the player that just reconnected
	/// \param opponent - name of the opponent that is still waiting in the game
	void addPlayersToGameRoom(std::string player, std::string opponent);

	/// Removes the client from the game room
	///
	/// \param player - name of the player that should be removed from the game room
	void removePlayerFromGameRoom(std::string player);


	// ----- MESSAGING -----

	/// Sends message to all clients (except sender) that are connected to the server
	///
	/// \param sendingClientName - username of the client that is sending the message
	/// \param data - message that is going to be send
	void broadcastToOtherClients(std::string sendingClientName, std::string data);

	/// Sends list of all online clients to the client given as parameter
	///
	/// \param client - reference to the client to whom the message will be sent to
	void updateAllOnlineClients(Client *client);

	/// Sends list of all busy clients to the client given as parameter
	///
	/// \param client - reference to the client to whom the message wil be sent to
	void updateAllBusyClients(Client *client);


	// ----- HELPER FUNCTIONS -----

	/// Converts IPv4 address to text representation
	///
	/// \param address - IPv4 address that should be converted
	/// \return Text representation of IPv4 address
	std::string ipToString(struct sockaddr_in address) const;

	/// Strips the new line character from the message
	///
	/// \param data - Message from which the new line characted should be removed
	void stripNewLineChar(char* data) const;

	/// Splits the message into array of tokens by separator
	///
	/// \param data - Message that should be splitted by separator into tokens
	/// \return Array of tokens from the message
	std::vector<std::string> tokenize(const std::string& data);

	/// Returns the type of the message by its header (first token)
	///
	/// \param header - first token of the message
	/// \return The type of the message that was received
	incomingMsgHeaders getMessageType(std::string header);

	/// Returns the opponent of the player given in parameter
	///
	/// \param player - username of the player for whom we search opponent
	/// \param lock - true if the list of players should be locked by mutex, false otherwise
	/// \return Username of the opponent
	std::string getOpponent(std::string player, bool lock);

	/// Checks if the player given as parameter is still in the game
	/// 
	/// \param player - username if the player that is beeing checked
	/// \return true if the player is still in the game, otherwise returns false
	bool isInTheGame(std::string player);

	/// Checks if the player is in the reconecting list
	///
	/// \param player - username of the player that is beeing checked
	/// \return true if the player is in the reconnecting list, otherwise false
	bool isReconnecting(std::string player);

	/// Check if the opponent of the player given as parameter is still in the game
	///
	/// \param palyer - username of the player whoms opponent would be tested
	/// \return  true if the players opponent is still in the game, otherwise false
	bool isOpponentInGame(std::string player);

	/// Gets the username of the player who sent the game request
	///
	/// \param receiver - username of the player who received the game request
	/// \return Username of the player who sent the request
	std::string getRequestPlayerName(std::string receiver);

};