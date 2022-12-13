#include "Socket.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


Socket::Socket() : Socket(AddressFamily::INET, Protocol::TCP) {}

Socket::Socket(AddressFamily addressFamily, Protocol protocol) {
    this->addressFamily = addressFamily;
    int fileDescriptor = socket(
        (int) addressFamily,
        (protocol == Socket::Protocol::TCP ? SOCK_STREAM : (protocol == Socket::Protocol::UDP ? SOCK_DGRAM : 0)),
        0
    );
    if (fileDescriptor < 0)
        throw SocketInitializationFailedException();
    this->fileDescriptor = fileDescriptor;
}

Socket::Socket(int fileDescriptor) {
    this->fileDescriptor = fileDescriptor;
}

Socket::~Socket() {
    this->close();
}

void Socket::bind(const char* ipAddressString, int portNumber) {
    sockaddr_storage ipEndpoint = Socket::getIpEndpoint(this->addressFamily, ipAddressString, portNumber);
    if (::bind(this->fileDescriptor, (sockaddr*) &ipEndpoint, sizeof(ipEndpoint)) < 0)
        throw UnableToBeBoundToEndpointException();
}

void Socket::listen(int backlog) {
    if (::listen(this->fileDescriptor, backlog) < 0)
        throw UnableToListenForConnectionsException();
}

Socket* Socket::accept() {
    int fileDescriptor = ::accept(this->fileDescriptor, nullptr, 0);
    if (fileDescriptor < 0)
        throw UnableToAcceptConnectionException();
    return new Socket(fileDescriptor);
}

Socket* Socket::accept(char* peerIpAddressString, int* peerPortNumber) {
    sockaddr_storage peerIpEndpoint;
    socklen_t peerIpEndpointSize = sizeof(peerIpEndpoint);
    int fileDescriptor = ::accept(this->fileDescriptor, (sockaddr*) &peerIpEndpoint, &peerIpEndpointSize);
    if (fileDescriptor < 0)
        throw UnableToAcceptConnectionException();
    Socket::parseIpEndpoint(peerIpEndpoint, peerIpAddressString, peerPortNumber);
    return new Socket(fileDescriptor);
}

void Socket::connect(const char* ipAddressString, int portNumber) {
    sockaddr_storage ipEndpoint = Socket::getIpEndpoint(this->addressFamily, ipAddressString, portNumber);
    if (::connect(this->fileDescriptor, (sockaddr*) &ipEndpoint, sizeof(ipEndpoint)) < 0)
        throw UnableToConnectToEndpointException();
}

int Socket::receive(char* receivedDataBuffer, size_t receivedDataBufferSize) {
    int bytesReceived = recv(this->fileDescriptor, receivedDataBuffer, receivedDataBufferSize, 0);
    if (bytesReceived < 0)
        throw UnableToReceiveDataException();
    return bytesReceived;
}

int Socket::receiveFrom(char* receivedDataBuffer, size_t receivedDataBufferSize, char* sourceIpAddressString, int* sourcePortNumber) {
    sockaddr_storage sourceIpEndpoint;
    socklen_t sourceIpEndpointSize = sizeof(sourceIpEndpoint);
    int bytesReceived = recvfrom(this->fileDescriptor, receivedDataBuffer, receivedDataBufferSize, 0, (sockaddr*) &sourceIpEndpoint, &sourceIpEndpointSize);
    if (bytesReceived < 0)
        throw UnableToReceiveDataException();
    Socket::parseIpEndpoint(sourceIpEndpoint, sourceIpAddressString, sourcePortNumber);
    return bytesReceived;
}

void Socket::send(const char* data, size_t dataSize) {
    if (::send(this->fileDescriptor, data, dataSize, 0) < 0)
        throw UnableToSendDataException();
}

void Socket::sendTo(const char* data, size_t dataSize, const char* destinationIpAddressString, int destinationPortNumber) {
    sockaddr_storage destinationIpEndpoint = Socket::getIpEndpoint(this->addressFamily, destinationIpAddressString, destinationPortNumber);
    if (::sendto(this->fileDescriptor, data, dataSize, 0, (sockaddr*) &destinationIpEndpoint, sizeof(destinationIpEndpoint)) < 0)
        throw UnableToSendDataException();
}

void Socket::shutdown(ShutdownManner shutdownManner) {
    if (::shutdown(this->fileDescriptor, (int) shutdownManner) < 0)
        throw UnableToShutdownException();
}

void Socket::close() {
    try {
        this->shutdown(ShutdownManner::SHUTDOWN_ALL);
    } catch (exception e) {}
    ::close(this->fileDescriptor);
}

// This function is used for creating a 'sockaddr_storage' struct from an IP-address string and a port number.
sockaddr_storage Socket::getIpEndpoint(AddressFamily addressFamily, const char* ipAddressString, int portNumber) {
    sockaddr_storage ipEndpoint;
    ipEndpoint.ss_family = (int) addressFamily;
    in6_addr ipAddress; // This struct is big enough to hold both IPv4 and IPv6 addresses.
    if (inet_pton((int) addressFamily, ipAddressString, &ipAddress) != 1)
        throw InvalidIpAddressException();
    switch (addressFamily) {
        case INET:
            ((sockaddr_in*) &ipEndpoint)->sin_addr = *((in_addr*) &ipAddress);
            ((sockaddr_in*) &ipEndpoint)->sin_port = htons(portNumber);
            break;
        case INET6:
            ((sockaddr_in6*) &ipEndpoint)->sin6_addr = ipAddress;
            ((sockaddr_in6*) &ipEndpoint)->sin6_port = htons(portNumber);
            break;
    }
    return ipEndpoint;
}

// This function is used for parsing an IP-address string and a port number from a 'sockaddr_storage' struct.
void Socket::parseIpEndpoint(sockaddr_storage ipEndpoint, char* ipAddressString, int* portNumber) {
    AddressFamily addressFamily = (AddressFamily) ipEndpoint.ss_family;
    in6_addr ipAddress; // This struct is big enough to hold both IPv4 and IPv6 addresses.
    switch (addressFamily) {
        case INET:
            *((in_addr*) &ipAddress) = ((sockaddr_in*) &ipEndpoint)->sin_addr;
            *portNumber = ntohs(((sockaddr_in*) &ipEndpoint)->sin_port);
            break;
        case INET6:
            *((in6_addr*) &ipAddress) = ((sockaddr_in6*) &ipEndpoint)->sin6_addr;
            *portNumber = ntohs(((sockaddr_in6*) &ipEndpoint)->sin6_port);
            break;
    }
    if (inet_ntop((int) addressFamily, &ipAddress, ipAddressString, INET6_ADDRSTRLEN) == nullptr)
        throw InvalidIpEndpointException();
}
