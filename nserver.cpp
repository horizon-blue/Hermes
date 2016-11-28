#include <ncurses.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cctype>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "nserver.h"
#include "nsocket.h"
#include "nutil.h"

using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::thread;

// mapping file names to list of client
std::unordered_map<string, vector<Socket>> client_map;
vector<string> file_list;
ServerSocket self;

int main(int argc, char** argv) {
    cout << "Welcome to mKilo server. :)" << endl;
    if(argc < 2) {
        cerr << "Usage: " << argv[0]
             << " <port number> [max clients = 10] [file path = ./_files/]"
             << endl;
        return 1;
    }
    std::signal(SIGINT, int_handler);

    cout << "Press Ctrl+C to quit." << endl;
    self.set_port(argv[1]);
    if(argc > 2) {
        for(int i = 0; argv[2][i]; ++i)
            if(!std::isdigit(argv[2][i])) {
                cerr << "Number of connections must be a number." << endl;
                return 1;
            }
        int mc = std::atoi(argv[2]);
        if(mc < 0 || mc > 10) {
            cerr << "Invalid number of connection." << endl;
            return 1;
        }
        self.set_max_connection(mc);
    }

    if(argc > 3)
        file_list = std::move(get_file_list(argv[3]));
    else
        file_list = std::move(get_file_list("./_files/"));

#ifdef DEBUG
    cout << "Avaliable files:" << endl;
    for(const string& s : file_list)
        cout << s << endl;
#endif

    self.connect();

    int retval = run_server();

    cout << "Exiting mKilo server.\n"
         << "Goodbye." << endl;
    return retval;
}

int run_server() {
    // main part of server program
    vector<thread> working_threads;
    return 0;
}