#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cinttypes>
#include <cstring>
#include <string>
#include <utility>

#include "socket.h"
#include "util.h"

using std::string;
using std::int32_t;

const size_t Socket::MESSAGE_SIZE_DIGITS;

Socket::Socket(const Socket& other)
    : Socket(other.socket, other.ip, other.port) {
    is_connected = other.is_connected;
    memcpy(&info, &(other.info), sizeof(info));
}

Socket& Socket::operator=(Socket&& other) {
    socket       = other.socket;
    ip           = std::move(other.ip);
    port         = std::move(other.port);
    is_connected = other.is_connected;
    std::memmove(&info, &(other.info), sizeof(info));
    return *this;
}

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

ssize_t Socket::send(const string& message, int command_type) {
    string encrypted;
    encrypted.push_back(static_cast<char>(command_type));
    encrypted.append(std::move(base64_encode(message)));
    int32_t len = htonl(encrypted.size() - 1);
    if(sen(reinterpret_cast<char*>(&len), MESSAGE_SIZE_DIGITS) < 0)
        return -1;
    ssize_t retval = sen(encrypted);
    PERROR("Sending " << encrypted << " of size " << ntohl(len));
    return retval;
}

ssize_t Socket::broadcast(const string& message,
                          const std::vector<int>& client_list,
                          int command_type) {
    string encrypted;
    encrypted.push_back(static_cast<char>(command_type));
    encrypted.append(std::move(base64_encode(message)));
    int32_t len    = htonl(encrypted.size() - 1);
    ssize_t retval = 0;
    for(const int& s : client_list) {
        if(sen(reinterpret_cast<char*>(&len), MESSAGE_SIZE_DIGITS, s) < 0)
            return -1;
        retval = sen(encrypted, s);
        if(retval <= 0)
            return retval;
    }
    return retval;
}

ssize_t Socket::sen(const string& message, size_t len, int s) {
    if(s == -1)
        s = socket;
    if(!is_connected)
        return -1;
    if(!len) {
        len = message.size();
    }
    size_t sent = 0;
    while(sent < len) {
        ssize_t temp = ::send(s, message.substr(sent).c_str(), len - sent, 0);
        if(temp < 0)
            return temp;
        if(temp == 0)
            return sent;
        sent += temp;
    }
    return sent;
}

ssize_t Socket::sen(const char* const message, size_t len, int s) {
    if(s == -1)
        s = socket;
    if(!is_connected)
        return -1;
    if(!len)
        return 0;
    size_t sent = 0;

    while(sent < len) {
        ssize_t temp = ::send(s, message + sent, len - sent, 0);
        if(temp < 0)
            return temp;
        if(temp == 0)
            return sent;
        sent += temp;
    }
    return sent;
}

ssize_t Socket::receive(string& buffer, int& command_type) {

    int32_t len;
    if(recv(reinterpret_cast<char*>(&len), MESSAGE_SIZE_DIGITS) < 0)
        return -1;

    len = ntohl(len);
    PERROR("message size is " << len);

    char c;
    if(recv(&c, 1) < 0)
        return -1;  // get command type
    command_type = static_cast<int>(c);

    string temp;
    ssize_t retval = recv(temp, static_cast<size_t>(len));
    if(retval <= 0)
        return retval;
    buffer = std::move(base64_decode(std::move(temp)));
    // buffer = std::move(temp);  // test
    return retval;
}

ssize_t Socket::recv(string& buffer, size_t len) {
    if(!is_connected)
        return -1;
    if(len > buffer.max_size()) {
        PERROR("Attempting to create a string of size"
               << len
               << "which is larger than str.max_size()");
        return -1;
    }
    // char* buffer = new char[len];
    string(len, '\0').swap(buffer);
    size_t got = 0;
    while(got < len) {
        ssize_t temp = ::recv(socket, &buffer[got], len - got, 0);
        if(temp < 0)
            return temp;
        if(temp == 0) {
            break;
        }
        got += temp;
    }
    for(size_t i = 0; i < buffer.size(); ++i)
        if(buffer[i] == '\0') {
            buffer.resize(i);
            break;
        }
    return got;
}

ssize_t Socket::recv(char* buffer, size_t len) {
    if(!is_connected)
        return -1;
    // char* buffer = new char[len];

    size_t got = 0;
    while(got < len) {
        ssize_t temp = ::recv(socket, buffer + got, len - got, 0);
        if(temp < 0)
            return temp;
        if(temp == 0) {
            break;
        }
        got += temp;
    }
    // buffer[got] = '\0';

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