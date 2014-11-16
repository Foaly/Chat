#include <iostream>
#include <iomanip>

#include <cstring>
#include <cerrno>

#include <arpa/inet.h>

#include <ifaddrs.h>
#include <unistd.h>

#include "../shared/Message.hpp"

int main() {
    const uint16_t port = 39999;

    // create a IPv6, datagram (UDP) socket
    int socketFileDescriptor = socket(PF_INET6, SOCK_DGRAM, 0);

/** Try this on OSX and test if IPv4 works without it.*/
//    const int no = 0;
//    setsockopt(socketFileDescriptor, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no));

    if (socketFileDescriptor == -1) {
        std::cout << "Could not create a socket. Error: " << std::strerror(errno) << std::endl;
        return 1;
    }

    /* set up socket */
    struct sockaddr_in6 receivingSocket;
    memset(&receivingSocket, 0, sizeof(receivingSocket));       // clear memory
    receivingSocket.sin6_family = AF_INET6;                     // address family. must be this for ipv4, or AF_INET6 for ipv6
    receivingSocket.sin6_addr = in6addr_any;                    // allows socket to work send and receive on any of the machines interfaces (which machine is used to send?)
    receivingSocket.sin6_port = htons(port);                    // the port we want to receive on

    /* bind our socket to the port  */
    int bindResult = bind(socketFileDescriptor, (struct sockaddr *)&receivingSocket, sizeof(receivingSocket));  // ask OS to let us use the socket

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

    std::cout << "IP addresses for " << hostname << ":" << std::endl << std::endl;

    struct ifaddrs *networkInterfaceList, *p;

    // get information about the network interfaces
    if (getifaddrs (&networkInterfaceList)) {
        std::cout << "Could not get information about the network interfaces. Error: " << std::strerror(errno) << std::endl;
        return 1;
    }

    // iterate over all the network interfaces we got and try to extract their IP address
    for (p = networkInterfaceList; p != NULL; p = p->ifa_next) {
        char serverIPAddress[INET6_ADDRSTRLEN];
        void *addr;
        std::string ipVersion;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ifa_addr->sa_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ifa_addr;
            addr = &(ipv4->sin_addr);
            ipVersion = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ifa_addr;
            addr = &(ipv6->sin6_addr);
            ipVersion = "IPv6";
        }

        // take the IP address of our hostname and convert it into a readable format
        inet_ntop(p->ifa_addr->sa_family, addr, serverIPAddress, sizeof(serverIPAddress));

        // sometime the IP address is empty so only print if there is one
        if (std::strlen(serverIPAddress) > 0)
            std::cout << "Interface: " << std::setw(6) << std::left << p->ifa_name << " " << ipVersion << ": " << serverIPAddress << std::endl;
    }

    freeifaddrs(networkInterfaceList); // free the linked list


    std::cout << std::endl << "Server is ready and listening on port " << port << std::endl;

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
