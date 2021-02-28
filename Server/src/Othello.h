#pragma once

#include <iostream>
#include <vector>
#include <cstring>
#include <thread>
#include <mutex>

#include "Server.h"


/// forward declaration
class Server;


/// \author Daniel Stuš
///
/// Class Othello handles the logic of the game Othello (also know as Reversi)
/// This class is used by the Server to controll the game
class Othello
{
public:

	/// Number of rows and columns (8x8grid)
	static const int GAME_BOARD_SIZE = 8;

	/// States of the game
	enum GameState
	{
		kPlayerOneWon,
		kPlayerTwoWon,
		kInProgress,
		kDraw
	};

private:

	/// States of each cell of the grid
	/// It can be owned with black chip (by player1), white chip (by player2) or be still free
	enum Chip {
		kFree,
		kBlack,
		kWhite
	};

	/// Reference to the server for sending messages to clients of the game
	Server* server;

	/// The game board
	Chip board[GAME_BOARD_SIZE][GAME_BOARD_SIZE];

	/// Username of the first player (client1)
	std::string player1;

	/// Username of the first player (client2)
	std::string player2;

	/// Indicator for who is on the move in the moment
	/// true - if it's whites move, false otherwise
	bool whites_move;

	/// Indicator if the game is waiting for one of the player to reconnect back
	bool waiting_for_reconnect;


public:
	/// Constructor of the class that creates an instance of the game
	///
	/// \param player1 - username of the first player (client1)
	/// \param player2 - username of the second player (client2)
	/// \param server - reference to the server used for sending messages to clients
	Othello(std::string player1, std::string player2, Server* server);

	/// Destructor fo the class
	~Othello();

	/// Makes one move - plays one turn
	/// Spawns chip in players colors on the x and y position,
	/// calculates and flips other chips and toggle the move to the opponent
	///
	/// \param x - x position of grid where the chip should be spawned
	/// \param y - y postiion of grid where the chip should be spawned
	GameState makeMove(int x, int y);

	/// Returns the current state of the game formatted for the recovery in client after reconnection
	///
	/// \param playerName - username of the player who is trying to reconnect
	/// \return current state of the game formatted for the clients recovery
	std::string getGameState(std::string playerName) const;

	/// Prints the state of the board to the console
	/// Used for debugging
	void printBoard();

private:

	/// Checks if the move is allowed
	///
	/// \param x - x position of grid
	/// \param y - y postiion of grid
	/// \param isWhite - true if it should check the validity of the move for white player, false if it should check the validity for black player
	/// \return true if the move is allowed, false if it's not
	bool isLegalMove(int x, int y, bool isWhite);

	/// Generates the starting chips in the middle of the board before playing
	void generateStartChips();

	/// Checks if the move is in bounds of the grid
	///
	/// \param x - x position of grid
	/// \param y - y position of grid
	/// \return true if the move is in bouds of the board, false otherwise
	bool isInBounds(int x, int y);

	/// Calculates all legal moves for player
	///
	/// \param isWhite - true if it should calculate valid moves of for white player, false if it should calculate valid moves for black player
	/// \return vector of x,y pairs of all current valid moves for the player
	std::vector<std::pair<int, int>> calculateLegalMoves(bool isWhite);

};