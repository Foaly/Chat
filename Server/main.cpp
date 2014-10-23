#include <iostream>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <cstring>
#include <cerrno>

#include "../shared/Message.hpp"

int main() {
    const uint16_t port = 39999;

    // create a IPv4, datagram (UDP) socket
    int socketFileDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

    if (socketFileDescriptor == -1) {
        std::cout << "Could not create a socket. Error: " << std::strerror(errno) << std::endl;
        return 1;
    }

    /* set up socket */
    struct sockaddr_in receivingSocket;
    memset(&receivingSocket, 0, sizeof(receivingSocket));       // clear memory
    receivingSocket.sin_family = AF_INET;		                // address family. must be this for ipv4, or AF_INET6 for ipv6
    receivingSocket.sin_addr.s_addr = htonl(INADDR_ANY);        // allows socket to work send and receive on any of the machines interfaces (which machine is used to send?)
    receivingSocket.sin_port = htons(port);		                // the port we want to receive on

    /* bind our socket to the port  */
    int bindResult = bind(socketFileDescriptor, (struct sockaddr *)&receivingSocket, sizeof(struct sockaddr));  // ask OS to let us use the socket

    if (bindResult == -1) {
        std::cout << "Could not bind the socket to port " << port << ". Error: " << std::strerror(errno) << std::endl;
        return 1;
    }

    std::cout << "Server is ready and listening on port " << port << std::endl;

    while (true) {
        /* create a buffer the size of a message */
        char buffer[sizeof(message_t)];
        struct sockaddr_in senderAddress;
        int senderAddressLength = sizeof(struct sockaddr);

        /* receive the data */
        int numberOfReceivedBytes = recvfrom(socketFileDescriptor,                      // socket we want to receive on
                                             buffer,                                    // buffer for the data
                                             sizeof (buffer),                           // size of the buffer
                                             0,                                         // flags
                                             (struct sockaddr *)&senderAddress,         // who sent the data
                                             (socklen_t *) &senderAddressLength);       // length of the struct

        if (numberOfReceivedBytes == -1) {
            std::cout << "Could not receive data. Error: " << std::strerror(errno) << std::endl;
            return 1;
        }
        // cast the received data into our message format
        message_t* receivedMessage = (message_t *) buffer;


        // convert the senders IP address into a string
        char ipAddress[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(senderAddress.sin_addr), ipAddress, INET_ADDRSTRLEN);

        // print the received message
        std::cout  << std::endl;
        std::cout << "The user \"" << receivedMessage->user_name << "\" (" << ipAddress << ":" << ntohs(senderAddress.sin_port) << ") sends " << numberOfReceivedBytes << " Bytes." << std::endl;
        std::cout << "Message #" << ntohl(receivedMessage->message_number) << ":" << std::endl;
        std::cout << receivedMessage->message << std::endl;
    }

    return 0;
}
