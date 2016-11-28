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
    while(sent < message.size()) {
        ssize_t temp = ::send(
            socket, message.substr(sent).c_str(), message.size() - sent, 0);
        if(temp < 0)
            return temp;
        if(temp == 0)
            return sent;
        sent += temp;
    }
    return sent;
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
        PERROR("listen() fails.")
        return false;
    }

    is_connected = true;
    return true;
}