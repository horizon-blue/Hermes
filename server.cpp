// Xiaoyan Wang
// Hermes server
#include <ncurses.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
// #include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "server.h"
#include "socket.h"
#include "util.h"

using std::string;
using std::list;
using std::cout;
using std::to_string;
using std::endl;
using std::vector;

// mapping file names to list of client
// use filename as index
// key "~" means the given client has not chose a file yet
std::unordered_map<string, vector<int>> client_map;
std::unordered_map<string, vector<LineEntry>> file_map;
std::mutex client_map_mutex;
std::mutex file_map_mutex;
vector<string> file_list;
vector<Socket> client_list;
string base_directory;
ServerSocket self;
volatile std::sig_atomic_t running = 1;
void int_handler(int sig) {
    self.disconnect();  // to prevent accept() from blocking
    running = 0;
}

int main(int argc, char** argv) {
    cout << "Welcome to Hermes server. :)" << endl;
    if(argc < 2) {
        cerr << "Usage: " << argv[0]
             << " [port number = 12345] [max clients = 10]\n"
             << "                 [file path = ./_files/]" << endl;
        self.set_port("12345");
    } else
        self.set_port(argv[1]);

    std::signal(SIGINT, int_handler);

    cout << "Press Ctrl+C to quit." << endl;

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
        base_directory = argv[3];
    else
        base_directory = "./_files/";

    file_list = std::move(get_file_list(base_directory.c_str()));

#ifdef DEBUG
    cout << "Avaliable files:\n------------\n";
    for(const string& s : file_list)
        cout << s << '\n';
    cout << "------------" << endl;
#endif

    self.connect();
    cout << "Server established on port " << self.get_port() << endl;

    int retval = run_server();

    // use \r to overwrite potential escape character
    cout << "\rClosing Hermes server...\n"
         << "Goodbye." << endl;
    return retval;
}

int run_server() {
    // main part of server program
    cout << "Waiting for connection..." << endl;
    std::thread client_thread(message_handler, 0);
    client_thread.detach();

    while(running)
        ;  // loop forever

    return 0;
}

void message_handler(size_t clientId) {
    // handling client
    PERROR("Starting thread " << clientId << "...");

    Socket client;
    if(!self.accept(client)) {
        PERROR("accept() fails");
        return;
    }

    // release another thread
    std::thread client_thread(message_handler, clientId + 1);
    client_thread.detach();

    ssize_t status = 1;
    string message;  // message received
    int command;     // command type
    string opened_file;

    cout << "Client " << clientId << " joined on " << client.get_ip() << endl;

    while(running && status) {
        status = client.receive(message, command);
        if(status <= 0) {
            PERROR("receive() fails");
            break;
        }
        cout << "Receive " << message << " with command " << command
             << " from client " << clientId << endl;
        switch(command) {
            case C_GET_REMOTE_FILE_LIST: {
                cout << "Sending file list to client " << clientId << endl;
                // sending number of files
                client.send(to_string(file_list.size()),
                            C_RESPONSE_REMOTE_FILE_LIST);
                // sending each file
                for(const string& filename : file_list)
                    client.send(filename);
                break;
            }
            case C_OPEN_FILE_REQUEST: {
                cout << "Client " << clientId << " ask to open file " << message
                     << endl;
                opened_file = message;
                // check if the file is already opened
                file_map_mutex.lock();
                if(file_map.find(message) == file_map.end())
                    server_open_file(message);
                const auto& file_vec = file_map.at(message);
                file_map_mutex.unlock();

                client.receive(message, command);
                int lines_to_send = std::min(std::stoi(message),
                                             static_cast<int>(file_vec.size()));

                client.send(to_string(lines_to_send), C_RESPONSE_FILE_INFO);
                client.send(to_string(file_vec.size()));
                for(int i = 0; i < lines_to_send;
                    ++i)  // send content of the line
                    client.send(file_vec[i].s);

                cout << "Sent " << lines_to_send << " lines." << endl;

                // client.send("This is just a placeholder for the actual
                // file.",
                //             C_RESPONSE_FILE_INFO);

                break;
            }
            case C_PUSH_LINE_BACK: {
                cout << "Push " << message << " of file " << opened_file
                     << " to client " << clientId << endl;
                const auto& file_vec = file_map.at(opened_file);
                size_t line_to_send  = std::stoi(message);
                // if(file_vec.size() < line_to_send)
                //     client.send("~", C_PUSH_LINE_BACK);  // shouldn't happen
                // else
                client.send(file_vec[line_to_send].s, C_PUSH_LINE_BACK);
                break;
            }
            case C_PUSH_LINE_FRONT: {
                cout << "Push " << message << " of file " << opened_file
                     << " to client " << clientId << endl;
                const auto& file_vec = file_map.at(opened_file);
                size_t line_to_send  = std::stoi(message);

                client.send(file_vec[line_to_send].s, C_PUSH_LINE_FRONT);
                break;
            }
        }
    }

    cout << "Client " << clientId << " left." << endl;
}

void server_open_file(const string& filename) {
    PERROR("Open file " << filename);
    std::ifstream fin(base_directory + filename);
    if(!fin.is_open())
        PERROR("Failed to open " << filename);
    // use the bracket operator to create a new entry
    auto& file_vec = file_map[filename];
    // read and store the entire file into the vector
    string temp;
    while(std::getline(fin, temp))
        file_vec.emplace_back(temp);
}