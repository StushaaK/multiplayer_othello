#include "Client.h"
#include "ErrorMessages.h"


const std::string Client::UNKNOWN= "UNKNOWN_USER";

Client::Client(int socket, std::string ip)
{
	this->socket = socket;
	this->ip = ip;
	this->state = kJoining;
	this->connected = true;
	username = UNKNOWN;

}

Client::~Client()
{
	close(socket);
}

void Client::send(std::string msg) const
{
	std::cout << "Sending a message to client" + toString() + ": '"+ msg + "'" << std::endl;
	msg += "\r\n";
	if (write(socket, msg.c_str(), strlen(msg.c_str())) < 0)
	{
		std::cout << SENDING_MESSAGE_ERROR << std::endl;
	}
}

int Client::getSocket() const
{
	return socket;
}

void Client::setState(State state)
{
	this->state = state;
}

Client::State Client::getState() const
{
	return state;
}

void Client::setUsername(std::string username)
{
	this->username = username;
}

std::string Client::getUsername() const
{
	return username;
}

void Client::setConnected(bool connected)
{
	this->connected = connected;
}

bool Client::isConnected() const
{
	return connected;
}

void Client::setPinged(bool pinged)
{
	this->pinged = pinged;
}

bool Client::getPinged()
{
	return pinged;
}


std::string Client::toString() const
{
	return "Client: '" + username + "'; Ip: '" + ip + "'";
}


