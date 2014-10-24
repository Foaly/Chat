#include <iostream>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <cstring>
#include <cerrno>
#include <cstdlib>

#include "ConsoleHelper.hpp"
#include "../shared/Message.hpp"

int main() {

    const std::string ipAddress = "127.0.0.1";
    const uint16_t port = 39999;


    // create a struct to store the servers address (IP address, port, etc.)
    struct sockaddr_in receiverAddress;
    // zero out the memory
    std::memset(&receiverAddress, 0, sizeof(receiverAddress));

    // convert the IP address from a string to an integer in the correct byte order and store it in the struct
    inet_pton(AF_INET, ipAddress.c_str(), &(receiverAddress.sin_addr));

    receiverAddress.sin_family = AF_INET;          // ipv4
    receiverAddress.sin_port = htons(port);        // set the receivers port


    // create a IPv4, datagram (UDP) socket
    int socketFileDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

    if (socketFileDescriptor == -1) {
        std::cout << "Could not create a socket. Error: " << std::strerror(errno) << std::endl;
        return 1;
    }

    /** The above could also work with getaddrinfo() and localhost */


    uint32_t messageNumber = 0;

    messagePacket messageToSend;

    // reserve some memory for the raw byte array
//    void* data = (unsigned char*)malloc(sizeof(messageToSend));
    unsigned char* data = new unsigned char[sizeof(messageToSend)];

    // get the users name
    ConsoleHelper consoleHelper;
    std::cout << "Welcome to the chat client!" << std::endl;
    std::cout << "Please input your name. (Press enter to confirm)" << std::endl;
    std::string name = consoleHelper.getLimitedInput(sizeof(messageToSend.user_name) - 1); // minus one for the \0 terminating the string
    std::cout << "Hello " << name << "!" << std::endl;

    while (true) {
        // increment the message number
        messageNumber = ntohl(messageNumber);
        messageNumber++;
        messageNumber = htonl(messageNumber); // convert memory layout to Network Byte Order

        // Assemble the message
        std::memset(&messageToSend, 0, sizeof(messageToSend));

        messageToSend.message_number = messageNumber;
        std::strncpy(messageToSend.user_name, name.c_str(), sizeof(messageToSend.user_name));

        // get the message from the user
        std::cout << std::endl << "Please input your message. (Press enter to send)" << std::endl;
        std::string messageString = consoleHelper.getLimitedInput(sizeof(messageToSend.message) - 1);

        // copy it into our struct
        std::strncpy(messageToSend.message, messageString.c_str(), sizeof(messageToSend.message));


        // convert the message into a raw byte array
        std::memcpy(data, &messageToSend, sizeof(messageToSend));

        // sent the data
        int numberOfSentBytes = sendto(socketFileDescriptor,                    // the socket we want to use
                                       data,                                    // the data we want to send
                                       sizeof(messageToSend),                   // the length of the data
                                       0,                                       // flags
                                       (struct sockaddr *) &receiverAddress,    // where to send it to
                                       sizeof(receiverAddress));                // address length

        if (numberOfSentBytes == -1) {
            std::cout << "Could not send data. Error: " << std::strerror(errno) << std::endl;
            return 1;
        }

    }

    // free the memory we reserved for the data
//    free(data);
    delete[] data;

    return 0;
}
