#ifndef __NSERVER_H__
#define __NSERVER_H__

#include <iostream>
#include <mutex>
#include <string>
#include "socket.h"
using std::endl;
using std::cout;
using std::string;

void int_handler(int sig);
int run_server();
void message_handler(size_t clientId);
void server_open_file(const string& filename);

struct LineEntry {
    LineEntry() = default;
    LineEntry(const LineEntry& other) : s(other.s) {}
    LineEntry(const string& line) : s(line) {}
    operator string&() { return s; }
    std::mutex m;
    string s;
};


#endif