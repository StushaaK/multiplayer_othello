#pragma once

#include <iostream>
#include <unistd.h>
#include <cstring>

/// \author Daniel Stuš A17B0354P
///
/// Class representing client
/// It holds basic information about client as well as the state
class Client
{
public:

	/// State of username before receiving CNAME message
	static const std::string UNKNOWN;

	/// All states of the client that he can be find in
	enum State 
	{
		kJoining,
		kLobby,
		kSentRq,
		kRecvRq,
		kPlaying, 
		kEnd
	};


private:

	/// Socket for communication with the client
	int socket;

	/// Ip of the client
	std::string ip;

	/// Username of the client
	std::string username;

	/// Current state of the client
	State state;

	/// Indicates if the client is still connected to the server. Used to determine if the server should listen for incoming messages.
	bool connected;

	/// Indicates if the client pinged the server in reserved time (10 sec)
	bool pinged;

public:

	/// Constructor of the client class
	///
	/// \param socket - the client uses for communication with the server
	/// \param ip - IPv4 address of the client
	Client(int socket, std::string ip);

	/// Destructor of the class
	/// closes the socket connection
	~Client();

	/// Getter for client socket
	///
	/// \return the socket of the client
	int getSocket() const;

	/// Sends the message to the client
	///
	/// \param msg - message to be sent
	void send(std::string msg) const;

	/// Sets the current state of the client
	///
	/// \param state - state that the client should be set to
	void setState(State state);

	/// Gets the current state of the client
	/// 
	/// \return the current state of the client
	State getState() const;

	/// Sets the username of the client
	///
	/// \param username - username that the client's username should be set to
	void setUsername(std::string username);

	/// Gets the username of the client
	///
	/// \return the usernam of the client
	std::string getUsername() const;

	/// Returns true or false whether the client is still connected
	/// and the main thread should continue running
	///
	/// \return true if the main thread should be running. Othrewise false
	bool isConnected() const;

	/// Sets the connetion state of the client
	/// Used to determine if the main thread should be running
	/// 
	/// \param connected - true or false whether the main thread should be running
	void setConnected(bool connected); 

	/// Sets the state whether the server received ping message from the client
	///
	/// \param pinged - true/false if the server received ping message from the client
	void setPinged(bool pinged);

	/// Gets the state whether the server received ping message from the client
	///
	/// \return true, if the server received ping message from the client. Otherwise false
	bool getPinged();

	/// Text representation of the client
	std::string toString() const;
};