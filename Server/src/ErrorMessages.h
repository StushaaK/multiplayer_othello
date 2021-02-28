#pragma once

#include <iostream>

/// \author Daniel Stuš
///
/// Error messages used in the server

const std::string SENDING_MESSAGE_ERROR = "There was an error while sending a message to the client!";
const std::string CREATING_FILE_DESCRIPTOR_ERROR = "Creating of the socket file descriptor failed";
const std::string ATTACHING_SOCKET_DESCRIPTOR_ERROR = "Attaching of the socket to the port failed";
const std::string BINDING_SERVER_DESCRIPTOR_ERROR = "There was an error during binding of the server";
const std::string LISTENING_FOR_CONNECTIONS_ERROR = "There was an error during listening for incoming connections";
const std::string ACCEPTING_CONNECTION_ERROR = "There was an error while accepting the connection";