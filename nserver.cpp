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

// mapping file names to list of client
// use filename as index
// key "~" means the given client has not chose a file yet
std::unordered_map<string, vector<size_t>> client_indexs;
std::mutex global_mutex;
vector<string> file_list;
vector<Socket> client_list;
ServerSocket self;
volatile std::sig_atomic_t running = 1;
void int_handler(int sig) {
    self.disconnect();  // to prevent accept() from blocking
    running = 0;
}

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
    client_list.resize(self.get_max_connection());

    if(argc > 3)
        file_list = std::move(get_file_list(argv[3]));
    else
        file_list = std::move(get_file_list("./_files/"));

#ifdef DEBUG
    cout << "Avaliable files:\n";
    for(const string& s : file_list)
        cout << s << '\n';
    cout << std::flush;
#endif

    self.connect();

    int retval = run_server();

    // use \r to overwrite potential escape character
    cout << "\rClosing mKilo server...\n"
         << "Goodbye." << endl;
    return retval;
}

int run_server() {
    // main part of server program
    vector<std::thread> working_threads;

    cout << "Waiting for connection..." << endl;

    while(running) {
        Socket client;
        if(!self.accept(client)) {
            PERROR("accept() fails");
            return 1;
        }
        // Add client to list
        global_mutex.lock();
        // find the first avaliable client that is not connected
        for(size_t i = 0; i < client_list.size(); ++i)
            if(!client_list[i].isconnected()) {
                client_list[i] = std::move(client);
                client_indexs["~"].push_back(i);
                break;
            }
        global_mutex.unlock();
        cout << "Client joined on " << client.get_ip() << endl;
        // // testing
        // string message;
        // client.receive(message, 18);
        // cout << "Receive " << message << " from " << client.get_ip() << endl;
    }
    for(std::thread& t : working_threads)
        t.join();

    return 0;
}