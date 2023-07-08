#include "Socket.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


Socket::Socket() : Socket(AddressFamily::INET, Protocol::TCP) {}

Socket::Socket(AddressFamily addressFamily, Protocol protocol) {
    this->addressFamily = addressFamily;
    int fileDescriptor = socket(static_cast<int>(addressFamily), (protocol == Protocol::TCP ? SOCK_STREAM : (protocol == Protocol::UDP ? SOCK_DGRAM : 0)), 0);
    if (fileDescriptor < 0)
        throw SocketInitializationFailedException();
    this->fileDescriptor = fileDescriptor;
}

Socket::Socket(int fileDescriptor) {
    this->fileDescriptor = fileDescriptor;
}

Socket::~Socket() {
    close();
}

void Socket::bind(const char* ipAddressString, int portNumber) const {
    sockaddr_storage ipEndpoint = getIpEndpoint(addressFamily, ipAddressString, portNumber);
    if (::bind(fileDescriptor, (sockaddr*) &ipEndpoint, sizeof(ipEndpoint)) < 0)
        throw UnableToBeBoundToEndpointException();
}

void Socket::listen(int backlog) const {
    if (::listen(fileDescriptor, backlog) < 0)
        throw UnableToListenForConnectionsException();
}

Socket* Socket::accept() const {
    int fileDescriptor = ::accept(this->fileDescriptor, nullptr, 0);
    if (fileDescriptor < 0)
        throw UnableToAcceptConnectionException();
    return new Socket(fileDescriptor);
}

Socket* Socket::accept(char* peerIpAddressString, int* peerPortNumber) const {
    sockaddr_storage peerIpEndpoint;
    socklen_t peerIpEndpointSize = sizeof(peerIpEndpoint);
    int fileDescriptor = ::accept(this->fileDescriptor, (sockaddr*) &peerIpEndpoint, &peerIpEndpointSize);
    if (fileDescriptor < 0)
        throw UnableToAcceptConnectionException();
    parseIpEndpoint(peerIpEndpoint, peerIpAddressString, peerPortNumber);
    return new Socket(fileDescriptor);
}

void Socket::connect(const char* ipAddressString, int portNumber) const {
    sockaddr_storage ipEndpoint = getIpEndpoint(addressFamily, ipAddressString, portNumber);
    if (::connect(fileDescriptor, (sockaddr*) &ipEndpoint, sizeof(ipEndpoint)) < 0)
        throw UnableToConnectToEndpointException();
}

int Socket::receive(char* receivedDataBuffer, size_t receivedDataBufferSize) const {
    int bytesReceived = recv(fileDescriptor, receivedDataBuffer, receivedDataBufferSize, 0);
    if (bytesReceived < 0)
        throw UnableToReceiveDataException();
    return bytesReceived;
}

int Socket::receiveFrom(char* receivedDataBuffer, size_t receivedDataBufferSize, char* sourceIpAddressString, int* sourcePortNumber) const {
    sockaddr_storage sourceIpEndpoint;
    socklen_t sourceIpEndpointSize = sizeof(sourceIpEndpoint);
    int bytesReceived = recvfrom(fileDescriptor, receivedDataBuffer, receivedDataBufferSize, 0, (sockaddr*) &sourceIpEndpoint, &sourceIpEndpointSize);
    if (bytesReceived < 0)
        throw UnableToReceiveDataException();
    parseIpEndpoint(sourceIpEndpoint, sourceIpAddressString, sourcePortNumber);
    return bytesReceived;
}

void Socket::send(const char* data, size_t dataSize) const {
    if (::send(fileDescriptor, data, dataSize, 0) < 0)
        throw UnableToSendDataException();
}

void Socket::sendTo(const char* data, size_t dataSize, const char* destinationIpAddressString, int destinationPortNumber) const {
    sockaddr_storage destinationIpEndpoint = getIpEndpoint(addressFamily, destinationIpAddressString, destinationPortNumber);
    if (::sendto(fileDescriptor, data, dataSize, 0, (sockaddr*) &destinationIpEndpoint, sizeof(destinationIpEndpoint)) < 0)
        throw UnableToSendDataException();
}

void Socket::shutdown(ShutdownManner shutdownManner) const {
    if (::shutdown(fileDescriptor, static_cast<int>(shutdownManner)) < 0)
        throw UnableToShutdownException();
}

void Socket::close() const {
    try {
        shutdown(ShutdownManner::SHUTDOWN_ALL);
    } catch (exception e) {}
    ::close(fileDescriptor);
}

// This function is used for creating a 'sockaddr_storage' struct from an IP-address string and a port number.
sockaddr_storage Socket::getIpEndpoint(AddressFamily addressFamily, const char* ipAddressString, int portNumber) {
    sockaddr_storage ipEndpoint;
    ipEndpoint.ss_family = static_cast<int>(addressFamily);
    in6_addr ipAddress; // This struct is big enough to hold both IPv4 and IPv6 addresses.
    if (inet_pton(static_cast<int>(addressFamily), ipAddressString, &ipAddress) != 1)
        throw InvalidIpAddressException();
    switch (addressFamily) {
        case AddressFamily::INET:
            ((sockaddr_in*) &ipEndpoint)->sin_addr = *((in_addr*) &ipAddress);
            ((sockaddr_in*) &ipEndpoint)->sin_port = htons(portNumber);
            break;
        case AddressFamily::INET6:
            ((sockaddr_in6*) &ipEndpoint)->sin6_addr = ipAddress;
            ((sockaddr_in6*) &ipEndpoint)->sin6_port = htons(portNumber);
            break;
    }
    return ipEndpoint;
}

// This function is used for parsing an IP-address string and a port number from a 'sockaddr_storage' struct.
void Socket::parseIpEndpoint(sockaddr_storage ipEndpoint, char* ipAddressString, int* portNumber) {
    AddressFamily addressFamily = static_cast<AddressFamily>(ipEndpoint.ss_family);
    in6_addr ipAddress; // This struct is big enough to hold both IPv4 and IPv6 addresses.
    switch (addressFamily) {
        case AddressFamily::INET:
            *((in_addr*) &ipAddress) = ((sockaddr_in*) &ipEndpoint)->sin_addr;
            *portNumber = ntohs(((sockaddr_in*) &ipEndpoint)->sin_port);
            break;
        case AddressFamily::INET6:
            *((in6_addr*) &ipAddress) = ((sockaddr_in6*) &ipEndpoint)->sin6_addr;
            *portNumber = ntohs(((sockaddr_in6*) &ipEndpoint)->sin6_port);
            break;
    }
    if (inet_ntop(static_cast<int>(addressFamily), &ipAddress, ipAddressString, INET6_ADDRSTRLEN) == nullptr)
        throw InvalidIpEndpointException();
}
