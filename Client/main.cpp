#include <iostream>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <cstring>
#include <cerrno>
#include <cstdlib>

#include "ConsoleHelper.hpp"
#include "../shared/Message.hpp"

int main() {
    // print a welcome message
    std::cout << "Welcome to the chat client!" << std::endl;

    // get the IP address the user want to send to (servers IP)
    std::cout << "Please input the IP address or hostname of the server." << std::endl;
    std::string ipAddress;
    std::getline(std::cin, ipAddress);

    // id no IP was entered use localhost
    if (ipAddress.empty()) {
        std::cout << "Empty IP! 'localhost' (127.0.0.1) is used." << std::endl;
        ipAddress = "127.0.0.1";
    }

    const char* port = "39999"; // the servers port number

    struct addrinfo hints, *serverInfo, *server;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use AF_INET (IPv4) or AF_INET6 (IPv6) to force version
    hints.ai_socktype = SOCK_DGRAM;

    // try to get a list of server responding to our hostname
    int rv;
    if ((rv = getaddrinfo(ipAddress.c_str(), port, &hints, &serverInfo)) != 0) {
        std::cout << "Failed to get information about the host \"" << ipAddress << "\". Error: " << gai_strerror(rv) << std::endl;
        return 1;
    }

    int socketFileDescriptor;

    // iterate over all the servers and try to connect to the first one we can
    for(server = serverInfo; server != NULL; server = server->ai_next) {
        // try to create a socket
        socketFileDescriptor = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

        if (socketFileDescriptor == -1) {
            std::cout << "Could not create a socket. Error: " << std::strerror(errno) << std::endl;
            continue;
        }

        break; // if we get here, we must have connected successfully
    }

    if (server == NULL) {
        // looped off the end of the list with no connection
        std::cout << "Failed to connect to a server." << std::endl;
        return 1;
    }


    uint32_t messageNumber = 0;

    messagePacket messageToSend;

    // get the users name
    ConsoleHelper consoleHelper;
    std::cout << "Please input your name. (Press enter to confirm)" << std::endl;
    std::string name;
    const bool nameInputWasSuccessful = consoleHelper.getLimitedInput(name, sizeof(messageToSend.user_name) - 1); // minus one for the \0 terminating the string

    // if the input was canceled exit
    if (!nameInputWasSuccessful) {
        std::cout << "Good Bye!" << std::endl;
        return 0;
    }
    std::cout << "Hello " << name << "!" << std::endl;

    // reserve some memory for the raw byte array
//    void* data = (unsigned char*)malloc(sizeof(messageToSend));
    unsigned char* data = new unsigned char[sizeof(messageToSend)];

    while (true) {
        // increment the message number
        messageNumber = ntohl(messageNumber);
        messageNumber++;
        messageNumber = htonl(messageNumber); // convert memory layout to Network Byte Order

        // zero data array
        std::memset(data, 0, sizeof(messageToSend));

        // Assemble the message
        std::memset(&messageToSend, 0, sizeof(messageToSend));

        messageToSend.message_number = messageNumber;
        std::strncpy(messageToSend.user_name, name.c_str(), sizeof(messageToSend.user_name));

        // get the message from the user
        std::cout << std::endl << "Please input your message. (Press enter to send)" << std::endl;
        std::string messageString;
        bool messageInputWasSuccessful = consoleHelper.getLimitedInput(messageString, sizeof(messageToSend.message) - 1);

        // if the input was canceled leave the loop
        if(!messageInputWasSuccessful) {
            break;
        }

        // copy it into our struct
        std::strncpy(messageToSend.message, messageString.c_str(), std::strlen(messageString.c_str()));


        // convert the message into a raw byte array
        std::memcpy(data, &messageToSend, sizeof(messageToSend));

        // sent the data
        int numberOfSentBytes = sendto(socketFileDescriptor,                    // the socket we want to use
                                       data,                                    // the data we want to send
                                       sizeof(messageToSend),                   // the length of the data
                                       0,                                       // flags
                                       server->ai_addr,                         // servers address (where to send it to)
                                       server->ai_addrlen );                    // address length

        if (numberOfSentBytes == -1) {
            std::cout << "Could not send data. Error: " << std::strerror(errno) << std::endl;
            return 1;
        }

    }

    std::cout << std::endl << "Chat is closing." << std::endl;
    std::cout << "Goody Bye!" << std::endl;
    
    // free the linked list containing the server information
    freeaddrinfo(serverInfo);

    // free the memory we reserved for the data
//    free(data);
    delete[] data;

    return 0;
}
