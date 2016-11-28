#include <ncurses.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <condition_variable>
#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "nserver.h"
#include "nsocket.h"
#include "nutil.h"

using std::string;
using std::cout;
using std::endl;
using std::vector;

// mapping file names to list of client
std::unordered_map<string, vector<Socket>> client_map;


int main(int argc, char **argv) {
    if(argc < 2) {
        cerr << "Usage: " << argv[0] << " <port number>" << endl;
        return 1;
    }
    std::signal(SIGINT, int_handler);
    cout << "Welcome to mKilo server. :)\n";
    cout << "Press Ctrl+C to quit." << endl;


    while(running) {
    }


    cout << "Exiting mKilo server.\n"
         << "Goodbye." << endl;
    return 0;
}
