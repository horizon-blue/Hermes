#ifndef __SOCKET_H__
#define __SOCKET_H__
// A wrapper class for C socket
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <string>
#include <vector>

#include "util.h"

using std::string;

class Socket {
public:
    Socket() = default;
    Socket(const string& _ip, const string& _port) : ip(_ip), port(_port) {}
    Socket(const int& _s, const string& _ip, const string& _port)
        : socket(_s), ip(_ip), port(_port) {}
    Socket(const Socket& other);
    ~Socket() {
        if(is_connected)
            disconnect();
    }
    // access functions
    const string& get_ip() const { return ip; }
    const string& get_port() const { return port; }
    int get_socket() const { return socket; }
    bool isconnected() const { return is_connected; }

    // modification functions
    void set_socket(const int& _s) { socket = _s; }
    void set_ip(const string& _ip) { ip = _ip; }
    void set_port(const string& _port) { port = _port; }
    void set_info(const sockaddr_in& i) {
        is_connected = true;
        std::memcpy(&info, &i, sizeof(info));
    }
    void clear_info() { std::memset(&info, 0, sizeof(info)); }

    ssize_t send(const string& message, int command_type = C_OTHER);
    ssize_t receive(string& buffer, int& command_type);

    // connection manipulation
    bool connect();
    bool disconnect();

    // could be used directly as socket
    operator int() const { return socket; }
    Socket& operator=(Socket&& other);

protected:
    // helper function
    ssize_t sen(const string& message, size_t len = 0, int s = -1);
    ssize_t sen(const char* const message, size_t len, int s = -1);
    ssize_t recv(char* buffer, size_t len);
    ssize_t recv(string& buffer, size_t len);

    // ispired by CS 241 chatroom lab
    static const size_t MESSAGE_SIZE_DIGITS = 4;

    int socket;
    string ip;
    string port;
    bool is_connected = false;
    sockaddr_in info;
};

class ServerSocket : public Socket {
public:
    ServerSocket() : Socket() {}
    ServerSocket(const string& _port, int _mc)
        : Socket("", _port), max_connection(_mc) {}
    // move constructor
    ServerSocket(ServerSocket&& other) = default;

    // overload connect for passive socket
    bool connect();
    bool accept(Socket& client);

    int get_num_client() const { return num_client; }
    int get_max_connection() const { return max_connection; }

    void set_max_connection(int mc) { max_connection = mc; }

    // move operator
    ServerSocket& operator=(ServerSocket&& other) = default;


private:
    int num_client     = 0;
    int max_connection = 10;
};

class ClientSocket : public Socket {
public:
    ssize_t broadcast(const string& message,
                      const std::vector<int>& client_list,
                      int command_type = C_OTHER);
};

#endif