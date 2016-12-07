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
std::list<ClientSocket> client_list;
std::unordered_map<string, vector<ServerLineEntry>> file_map;
std::mutex client_list_mutex;
std::mutex file_map_mutex;
vector<string> file_list;
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

    ClientSocket pre_client;
    if(!self.accept(pre_client)) {
        PERROR("accept() fails");
        return;
    }

    client_list_mutex.lock();
    // so that it we could easily delete client when it left
    client_list.push_back(pre_client);
    auto iter_self = --client_list.end();
    client_list_mutex.unlock();
    auto& client = *iter_self;  // for easy reference

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
        PERROR("Receive " << message << " with command " << command
                          << " from client "
                          << clientId);
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
                PERROR("Client " << clientId << " ask to open file "
                                 << message);
                client.filename = std::move(message);
                // check if the file is already opened
                file_map_mutex.lock();
                if(file_map.find(client.filename) == file_map.end())
                    server_open_file(client.filename);
                // const auto& file_vec = file_map.at(client.filename);
                client.file_vec = &(file_map.at(client.filename));
                file_map_mutex.unlock();

                client.receive(message, command);
                int lines_to_send =
                    std::min(std::stoul(message), client.file_vec->size());
                client.begloc      = 0;
                client.rownum      = lines_to_send;
                client.currloc     = 0;
                client.client_list = &client_list;
                client.send(to_string(lines_to_send), C_RESPONSE_FILE_INFO);
                client.send(to_string(client.file_vec->size()));
                // send contents line by line
                for(int i = 0; i < lines_to_send; ++i)
                    client.send(client[i]);
                client_list_mutex.lock();
                client.isready = true;
                client_list_mutex.unlock();
                PERROR("Sent " << lines_to_send << " lines.");

                // client.send("This is just a placeholder for the actual
                // file.",
                //             C_RESPONSE_FILE_INFO);

                break;
            }
            case C_PUSH_LINE_BACK: {
                PERROR("Push " << message << " of file " << client.filename
                               << " to client "
                               << clientId);
                size_t line_to_send = std::stoul(message);
                client.begloc       = line_to_send - client.rownum;
                client.currloc      = line_to_send;
                // if(file_vec.size() < line_to_send)
                //     client.send("~", C_PUSH_LINE_BACK);  // shouldn't happen
                // else
                client.send(client[line_to_send], C_PUSH_LINE_BACK);
                break;
            }
            case C_ADD_LINE_BACK: {
                size_t line_to_send = std::stoul(message);
                client.send(client[line_to_send], C_ADD_LINE_BACK);
                break;
            }
            case C_PUSH_LINE_FRONT: {
                PERROR("Push " << message << " of file " << opened_file
                               << " to client "
                               << clientId);
                size_t line_to_send = std::stoul(message);
                client.begloc       = line_to_send;
                client.currloc      = line_to_send;
                client.send(client[line_to_send], C_PUSH_LINE_FRONT);
                break;
            }
            case C_UPDATE_LINE_CONTENT: {
                if(!client.isediting)
                    break;
                // size_t line_to_update = std::stoi(message);
                // client.receive(message, command);
                client.broadcast(to_string(client.currloc),
                                 C_UPDATE_LINE_CONTENT);
                client.broadcast(client.update_line(std::move(message)),
                                 C_UPDATE_LINE_CONTENT);
                // TODO: broadcast change to all clients under this file
                break;
            }
            case C_SET_CURSOR_POS: {
                if(!client.isediting) {
                    client.currloc = std::stoul(message);
                    break;
                }

                // client[client.currloc].m.unlock();
                client.currloc = std::stoul(message);
                // client[client.currloc].m.lock();
                break;
            }
            case C_SWITCH_TO_BROWSING_MODE: {
                // client.currloc = std::stoul(message);
                if(client.isediting) {
                    client.isediting = false;
                    // client[client.currloc].m.unlock();
                }
                break;
            }
            case C_SWITCH_TO_EDITING_MODE: {
                // client.currloc = std::stoul(message);
                if(!client.isediting) {
                    client.isediting = true;
                    // client[client.currloc].m.lock();
                }
                break;
            }
            case C_SAVE_FILE: {
                // save the file and inform the client when we are done
                server_save_file(client.filename);
                client.send("", C_SAVE_FILE);
            }
            case C_INSERT_LINE: {
                if(!client.isediting)
                    break;
                // the line before breaking should have already been
                // updated... the line should be inserted after
                // current row

                client.insert_line(message);
                client.broadcast(to_string(client.currloc), C_INSERT_LINE);
                client.broadcast(message, C_INSERT_LINE);
            }
            case C_DELETE_LINE: {
                if(!client.isediting)
                    break;
                size_t line_to_delete = std::stoul(message);
                client.delete_line(line_to_delete);
                client.broadcast(message, C_DELETE_LINE);
            }
        }
    }

    cout << "Client " << clientId << " left." << endl;
    client_list_mutex.lock();
    client_list.erase(iter_self);
    client_list_mutex.unlock();
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
    while(std::getline(fin, temp)) {
        while(!temp.empty() && (temp.back() == '\n' || temp.back() == '\r' ||
                                temp.back() == '\0'))
            temp.pop_back();  // remove new lines and null characters
        file_vec.emplace_back(temp);
    }
}

void server_save_file(const string& filename) {
    PERROR("Saving file" << filename);
    std::ofstream fout(base_directory + filename);
    if(!fout.is_open())
        PERROR("Failed to save " << filename);
    // if the vector does not exist, we will simply create
    // an empty file
    auto& file_vec = file_map[filename];
    for(const string& s : file_vec)
        fout << s << '\n';
    fout << std::flush;
}