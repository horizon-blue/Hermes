#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <string>

#include "nsocket.h"
#include "nutil.h"

using std::string;

bool Socket::connect() {
    if(is_connected || ip == "" || port == "")
        return false;
    for(const char& c : port)
        if(!std::isdigit(c)) {
            port = "";
            return false;
        }

    if((socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        return false;
    std::memset(&info, 0, sizeof(info));  // Zero out addrinfo structure
    info.sin_family      = AF_INET;       // Internet address family
    info.sin_addr.s_addr = inet_addr(ip.c_str());
    info.sin_port        = htons(static_cast<unsigned short>(std::stol(port)));
    if(::connect(socket, reinterpret_cast<sockaddr*>(&info), sizeof(info)) <
       0) {
        close(socket);
        return false;
    }
    is_connected = true;
    return true;
}

bool Socket::disconnect() {
    if(!is_connected)
        return false;
    shutdown(socket, SHUT_RDWR);
    close(socket);
    is_connected = false;
    return true;
}

ssize_t Socket::send(const string& message) {
    if(!is_connected)
        return -1;
    size_t sent = 0;
    // since std::string does not include null terminator
    // explicitly plus one to include it
    while(sent < message.size() + 1) {
        ssize_t temp = ::send(
            socket, message.substr(sent).c_str(), message.size() - sent + 1, 0);
        if(temp < 0)
            return temp;
        if(temp == 0)
            return sent;
        sent += temp;
    }
    return sent;
}

ssize_t Socket::receive(string& message, size_t len) {
    if(!is_connected)
        return -1;
    char* buffer = new char[len];
    size_t got   = 0;
    while(got < len) {
        ssize_t temp = ::recv(socket, buffer + got, len - got, 0);
        if(temp < 0)
            return temp;
        if(temp == 0) {
            break;
        }
        got += temp;
    }
    buffer[got] = '\0';
    message     = buffer;
    delete[] buffer;
    return got;
}

bool ServerSocket::connect() {
    if(is_connected || port == "")
        return false;
    for(const char& c : port)
        if(!std::isdigit(c)) {
            port = "";
            return false;
        }

    if((socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        return false;

    int optval = 1;
    setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    std::memset(&info, 0, sizeof(info));  // Zero out addrinfo structure
    info.sin_family      = AF_INET;       // Internet address family
    info.sin_addr.s_addr = htonl(INADDR_ANY);
    info.sin_port        = htons(static_cast<unsigned short>(std::stol(port)));

    if(bind(socket, reinterpret_cast<sockaddr*>(&info), sizeof(info)) < 0) {
        close(socket);
        PERROR("bind() fails.");
        return false;
    }

    if(listen(socket, max_connection) < 0) {
        close(socket);
        PERROR("listen() fails.");
        return false;
    }

    is_connected = true;
    return true;
}

bool ServerSocket::accept(Socket& client) {
    if(client.isconnected() || num_client == max_connection)
        return false;
    sockaddr_in clientinfo;
    std::memset(&info, 0, sizeof(clientinfo));
    socklen_t clientAddrlen = sizeof(clientinfo);

    // using the regular accept
    int retval = ::accept(
        socket, reinterpret_cast<sockaddr*>(&clientinfo), &clientAddrlen);
    if(retval < 0) {
        PERROR("accept() fails.");
        return false;
    }
    client.set_info(clientinfo);
    client.set_socket(retval);
    client.set_ip(inet_ntoa(clientinfo.sin_addr));
    ++num_client;
    return true;
}