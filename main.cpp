#include <iostream>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <cstring>
#include <cerrno>
#include <cstdlib>

#include "ConsoleHelper.hpp"

typedef struct {
    uint32_t message_number;
    char user_name[8];
    char message[138];
} message_t;


int main() {

    ConsoleHelper consoleHelper;

    const std::string ipAddress = "127.0.0.1";
    const uint16_t port = 39999;


    // create a struct to store the servers address (IP address, port, etc.)
    struct sockaddr_in server;
    // zero out the memory
    std::memset(&server, 0, sizeof(server));

    // convert the IP address from a string to an integer in the correct byte order and store it in the struct
    inet_pton(AF_INET, ipAddress.c_str(), &(server.sin_addr));

    server.sin_family = AF_INET;                         // ipv4
    server.sin_port = htons(port);                // set port address (is this the sender's port or the receiver's port


    // create a IPv4, datagram (UDP) socket
    int socketFileDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

    if (socketFileDescriptor == -1) {
        std::cout << "Could not create a socket. Error: " << std::strerror(errno) << std::endl;
        return 1;
    }

    /** The above could also work with getaddrinfo() and localhost */


    uint32_t messageNumber = 1;
    messageNumber = htonl(messageNumber); // convert memory layout to Network Byte Order

    // Assemble the message
    message_t messageToSend;
    std::memset(&messageToSend, 0, sizeof(messageToSend));

    messageToSend.message_number = messageNumber;
    std::strncpy(messageToSend.user_name, "Max", sizeof(messageToSend.user_name));
    std::strncpy(messageToSend.message, "The Cheese is in the Toaster", sizeof(messageToSend.message));

    std::cout << "Size: " << sizeof(messageToSend) << std::endl;
    std::cout << "Size: " << sizeof(messageToSend.message) << std::endl;

    // convert the message into a raw byte array
    void* data = (unsigned char*)malloc(sizeof(messageToSend));
    std::memcpy(data, &messageToSend, sizeof(messageToSend));

    // sent the data
    int numberOfSentBytes = sendto(socketFileDescriptor,            // the socket we want to use
                                   data,                            // the data we want to send
                                   sizeof(messageToSend),           // the length of the data
                                   0,                               // flags
                                   (struct sockaddr *) &server,     // where to send it to
                                   sizeof(server));                 // address length

    if (numberOfSentBytes == -1) {
        std::cout << "Could not send data. Error: " << std::strerror(errno) << std::endl;
        return 1;
    }

    // free the memory we reserved for the data
    free(data);



//    std::cout << "Please input your name. (Press enter to confirm)" << std::endl;
//    std::string name = consoleHelper.getLimitedInput(8);
//    std::cout << "Hello " << name << "!" << std::endl;
//
//    std::cout << "Please input your message. (Press enter to send)" << std::endl;
//    std::string messageString = consoleHelper.getLimitedInput(137);
//
//    std::cout << messageString.length() << std::endl;

    return 0;
}

/** This is needed for the server */
//    char str[INET_ADDRSTRLEN];
//    // now get it back and print it
//    inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
//
//    std::cout << str << std::endl; // prints "192.0.2.33"
