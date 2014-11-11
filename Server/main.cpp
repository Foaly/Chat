#include <iostream>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netdb.h>
#include <ifaddrs.h>

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

    // get the hostname of the server
    char hostname[128];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        std::cout << "Could not get the hostname of the server. Error: " << std::strerror(errno) << std::endl;
        return 1;
    }

    struct addrinfo hints, *serverInfo, *p;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use AF_INET (IPv4) or AF_INET6 (IPv6) to force version
    hints.ai_socktype = SOCK_DGRAM;

    // try to get information about our hostname
    int rv;
    if ((rv = getaddrinfo(hostname, NULL, &hints, &serverInfo)) != 0) {
        std::cout << "Failed to get information about the host \"" << hostname << "\". Error: " << gai_strerror(rv) << std::endl;
        return 1;
    }

    std::cout << "IP addresses for " << hostname << ":" << std::endl;

    // iterate over all the infos we got and try to extract the IP address
    for(p = serverInfo; p != NULL; p = p->ai_next) {
        char serverIPAddress[INET6_ADDRSTRLEN];
        void *addr;
        std::string ipVersion;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipVersion = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipVersion = "IPv6";
        }

        // take the IP address of our hostname and convert it into a readable format
        inet_ntop(p->ai_family, addr, serverIPAddress, sizeof(serverIPAddress));
        std::cout << "  " << ipVersion << ": " << serverIPAddress << std::endl;
    }

    freeaddrinfo(serverInfo); // free the linked list

    // this lists all available interfaces might be a little better as the above is not working as expected...
//    struct ifaddrs *ifap, *ifa;
//    struct sockaddr_in *sa;
//    char *addr;
//
//    getifaddrs (&ifap);
//    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
//        if (ifa->ifa_addr->sa_family==AF_INET) {
//            sa = (struct sockaddr_in *) ifa->ifa_addr;
//            addr = inet_ntoa(sa->sin_addr);
//            printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
//        }
//    }
//
//    freeifaddrs(ifap);


    std::cout << "Server is ready and listening on port " << port << std::endl;

    while (true) {
        /* create a buffer the size of a message */
        char buffer[sizeof(messagePacket)];
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
        messagePacket* receivedMessage = (messagePacket *) buffer;


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
