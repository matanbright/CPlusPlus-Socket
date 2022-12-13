#if !defined(SOCKET_H)
#define SOCKET_H



#include <stdexcept>
#include <sys/socket.h>

using namespace std;


class Socket {

    public: class SocketInitializationFailedException : public exception {
        private: const char* const EXCEPTION_MESSAGE = "Error: Socket initialization has been failed!";
        public: const char* what() const noexcept override {
            return EXCEPTION_MESSAGE;
        }
    };
    
    public: class UnableToBeBoundToEndpointException : public exception {
        private: const char* const EXCEPTION_MESSAGE = "Error: The socket is unable to be bound to the specified endpoint!";
        public: const char* what() const noexcept override {
            return EXCEPTION_MESSAGE;
        }
    };

    public: class UnableToListenForConnectionsException : public exception {
        private: const char* const EXCEPTION_MESSAGE = "Error: The socket is unable to listen for connections!";
        public: const char* what() const noexcept override {
            return EXCEPTION_MESSAGE;
        }
    };

    public: class UnableToAcceptConnectionException : public exception {
        private: const char* const EXCEPTION_MESSAGE = "Error: The socket is unable to accept a connection!";
        public: const char* what() const noexcept override {
            return EXCEPTION_MESSAGE;
        }
    };

    public: class UnableToConnectToEndpointException : public exception {
        private: const char* const EXCEPTION_MESSAGE = "Error: The socket is unable to connect to the specified endpoint!";
        public: const char* what() const noexcept override {
            return EXCEPTION_MESSAGE;
        }
    };

    public: class UnableToReceiveDataException : public exception {
        private: const char* const EXCEPTION_MESSAGE = "Error: The socket is unable to receive data!";
        public: const char* what() const noexcept override {
            return EXCEPTION_MESSAGE;
        }
    };

    public: class UnableToSendDataException : public exception {
        private: const char* const EXCEPTION_MESSAGE = "Error: The socket is unable to send data!";
        public: const char* what() const noexcept override {
            return EXCEPTION_MESSAGE;
        }
    };

    public: class UnableToShutdownException : public exception {
        private: const char* const EXCEPTION_MESSAGE = "Error: The socket is unable to shutdown!";
        public: const char* what() const noexcept override {
            return EXCEPTION_MESSAGE;
        }
    };

    public: class InvalidIpAddressException : public exception {
        private: const char* const EXCEPTION_MESSAGE = "Error: The specified IP address is not valid for the selected address family!";
        public: const char* what() const noexcept override {
            return EXCEPTION_MESSAGE;
        }
    };

    public: class InvalidIpEndpointException : public exception {
        private: const char* const EXCEPTION_MESSAGE = "Error: The specified IP endpoint is not valid!";
        public: const char* what() const noexcept override {
            return EXCEPTION_MESSAGE;
        }
    };

    public: enum AddressFamily {
        INET = AF_INET,
        INET6 = AF_INET6
    };

    public: enum Protocol {
        TCP,
        UDP
    };

    public: enum ShutdownManner {
        SHUTDOWN_READ = SHUT_RD,
        SHUTDOWN_WRITE = SHUT_WR,
        SHUTDOWN_ALL = SHUT_RDWR
    };

    private: int fileDescriptor;
    private: AddressFamily addressFamily;

    public: Socket();
    public: Socket(AddressFamily addressFamily, Protocol protocol);
    private: Socket(int fileDescriptor);
    public: ~Socket();
    public: void bind(const char* ipAddressString, int portNumber);
    public: void listen(int backlog);
    public: Socket* accept();
    public: Socket* accept(char* peerIpAddressString, int* peerPortNumber);
    public: void connect(const char* ipAddressString, int portNumber);
    public: int receive(char* receivedDataBuffer, size_t receivedDataBufferSize);
    public: int receiveFrom(char* receivedDataBuffer, size_t receivedDataBufferSize, char* sourceIpAddressString, int* sourcePortNumber);
    public: void send(const char* data, size_t dataSize);
    public: void sendTo(const char* data, size_t dataSize, const char* destinationIpAddressString, int destinationPortNumber);
    public: void shutdown(ShutdownManner shutdownManner);
    public: void close();
    private: static sockaddr_storage getIpEndpoint(AddressFamily addressFamily, const char* ipAddressString, int portNumber);
    private: static void parseIpEndpoint(sockaddr_storage ipEndpoint, char* ipAddressString, int* portNumber);
};



#endif
