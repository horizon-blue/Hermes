#ifndef __NSOCKET_H__
#define __NSOCKET_H__
// A wrapper class for C socket
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <string>


using std::string;

class Socket {
public:
    Socket() = default;
    Socket(const string& _ip, const string& _port) : ip(_ip), port(_port) {}
    Socket(const int& _s, const string& _ip, const string& _port)
        : socket(_s), ip(_ip), port(_port) {}
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
    void clear_info() { std::memset(&info, 0, sizeof(info)); }

    // connection manipulation
    bool connect();
    bool disconnect();
    ssize_t send(const string& message);

    // could be used directly as socket
    operator int() const { return socket; }

protected:
    int socket;
    string ip;
    string port;
    bool is_connected = false;
    sockaddr_in info;
};

class ServerSocket : public Socket {
public:
    ServerSocket(const string& _port, int _mc)
        : Socket("", _port), max_connection(_mc) {}
    // overload connect for passive socket
    bool connect();

    int get_num_client() const { return num_client; }
    int get_max_connection() const { return max_connection; }

    void set_max_connection(int mc) { max_connection = mc; }

private:
    int num_client     = 0;
    int max_connection = 10;
};

#endif