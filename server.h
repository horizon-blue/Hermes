#ifndef __NSERVER_H__
#define __NSERVER_H__

#include <iostream>
#include <string>
#include "socket.h"
using std::endl;
using std::cout;
using std::string;

void int_handler(int sig);
int run_server();
void message_handler(size_t clientId);


#endif