#ifndef __NSERVER_H__
#define __NSERVER_H__

#include <climits>
#include <iostream>
#include <list>
#include <mutex>
#include <string>
#include "socket.h"
using std::endl;
using std::cout;
using std::string;
using std::list;

void int_handler(int sig);
int run_server();
void message_handler(size_t clientId);
void server_open_file(const string& filename);
void server_save_file(const string& filename);
// ssize_t broadcast(const list<ClientSocket>& client_list,
//                   const string& filename,
//                   const string& message,
//                   int command_type = C_OTHER,
//                   int index        = -1);


#endif