#ifndef __NSERVER_H__
#define __NSERVER_H__

#include <iostream>
#include <string>
using std::endl;
using std::cout;
using std::string;

volatile std::sig_atomic_t running = 1;
void int_handler(int sig) {
    running = 0;
}


#endif