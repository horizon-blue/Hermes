// Xiaoyan Wang
// A new text editor written from starch
// Based on C++ && ncurses
#include <ncurses.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "client.h"
#include "editor.h"
#include "socket.h"
#include "util.h"
#include "window.h"

using std::vector;
using std::to_string;
using std::list;
using std::string;
using std::lock;
// using std::mutex;
// using std::thread;
// using std::getchar;

int max_row, max_col;
volatile std::sig_atomic_t running = 0;
vector<string> file_list;
list<ClientLineEntry> file_contents;
std::mutex file_list_mutex;
std::mutex status_mutex;
std::condition_variable status_cv;
Socket server;  // contains socket, ip, port number, etc.
Editor editor;  // for windows info, etc.

int main() {
    // --------------- init --------------------
    initscr();             // setup ncurses screen
    raw();                 // enable raw mode so we can capture ctrl+c, etc.
    keypad(stdscr, true);  // to capture arrow key, etc.
    noecho();              // so that escape characters won't be printed
    getmaxyx(stdscr, max_row, max_col);  // get max windows size
    start_color();                       // enable coloring
    init_colors();                       // add color configurations
    std::signal(SIGSEGV, segfault_handler);

    // -------------- done init ----------------

    // ---------- establish connection ---------
    print_welcome_screen();  // let user enter ip and port
    // ---------- connection established -------

    // ---------- init editor ------------------

    // use multithread to handle message
    std::thread handler_thread(message_handler);
    run_editor();


    // for(char c = getch(); c != KEY_CTRL_Q; c = getch()) {
    //     // printw("%c", c);
    //     ;
    //     // refresh();
    // }
    // // getch();

    handler_thread.join();
    endwin();  // end curse mode and restore screen

    return 0;
}


void init_colors() {
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_RED);
}


void message_handler() {
    // TODO
    ssize_t status = 1;
    string message;  // message received
    int command;     // command received

    // getting file list before starting editor
    server.send("init", C_GET_REMOTE_FILE_LIST);
    if(server.receive(message, command) < 0) {
        mvprintw(0, max_col / 2 - 10, "receive() fails.");
        return;
    }
    int num_message = std::stoi(message);
    file_list.reserve(num_message);
    for(int i = 0; i < num_message; ++i) {
        server.receive(message, command);
        file_list.push_back(message);
    }

    running = 1;

    while(running && status) {
        status = server.receive(message, command);
        if(status <= 0) {
            PERROR("receive() fails.");
            running = 0;
            break;
        }

        switch(command) {
            case C_RESPONSE_FILE_INFO: {
                num_message = std::stoi(message);
                server.receive(message, command);
                editor.file.set_num_file_lines(std::stoi(message));
                for(int i = 0; i < num_message; ++i) {
                    server.receive(message, command);
                    file_contents.emplace_back(std::move(message), i);
                }
                running = 3;
                break;
            }
            case C_PUSH_LINE_BACK: {
                file_contents.pop_front();
                file_contents.emplace_back(std::move(message),
                                           file_contents.back().linenum + 1);
                running = 3;
                break;
            }
            case C_PUSH_LINE_FRONT: {
                file_contents.pop_back();
                file_contents.emplace_front(std::move(message),
                                            file_contents.front().linenum - 1);
                running = 3;
                break;
            }
        }
    }
}

void run_editor() {
    int y, x;
    getyx(stdscr, y, x);
    mvprintw(y + 1, max_col / 2 - 11, "Retrieving file list...");
    getyx(stdscr, y, x);
    while(!running)  // wait until file list is ready
        ;

    // init editor
    editor.init(max_row, max_col);
    erase();
    refresh();
    editor.status.print_filename("~");  // denotes that we haven't select file
    editor.status.print_status(
        "Press Enter to select a file. Press Ctrl+Q to quit.");
    editor.dir.print_filelist(file_list);
    int c;
    while(running) {
        while(running == S_DIR_MODE) {  // directory mode
            c = wgetch(editor.dir);
            switch(c) {
                case KEY_UP:
                    editor.dir.scroll_up(file_list);
                    break;
                case KEY_DOWN:
                    editor.dir.scroll_down(file_list);
                    break;
                case KEY_CTRL_Q:
                    endwin();
                    running = 0;
                    std::exit(0);
                case KEY_ENTER:
                case '\n':
                    editor.status.print_filename(
                        file_list[editor.dir.get_selection()]);
                    editor.switch_mode();
                    running = S_WAITING_MODE;
                    break;
            }
        }
        editor.status.print_status("Retrieving file contents...");
        // send file name and number of rows
        server.send(editor.status.get_filename(), C_OPEN_FILE_REQUEST);
        server.send(to_string(max_row - 2));
        while(running == S_WAITING_MODE)  // waiting for file content
            ;
        editor.status.print_status("Welcome to Hermes. Press Ctrl+Q to quit.");
        editor.file.set_file_content(&file_contents);
        editor.file.refresh_file_content(-1);
        // int y = 0;
        // for(const string& line : file_contents)
        //     editor.file.printline(line, y++);

        while(running == S_FILE_MODE) {  // file mode
            c = wgetch(editor.file);
            if(std::isprint(c)) {
                editor.file.insertchar(c);
                server.send(to_string(editor.file.get_row()),
                            C_UPDATE_LINE_CONTENT);
                server.send(editor.file.get_currline());

                // skip the following switch statement
                continue;
            }
            switch(c) {
                case KEY_CTRL_Q:
                    endwin();
                    running = 0;
                    std::exit(0);
                case KEY_UP:
                    if(editor.file.scroll_up() == -1) {
                        // retrieve previous line from server
                        server.send(
                            to_string(file_contents.front().linenum - 1),
                            C_PUSH_LINE_FRONT);
                        running = S_WAITING_MODE;
                        while(running == S_WAITING_MODE)
                            ;
                        editor.file.refresh_file_content(-1);
                    }
                    break;
                case KEY_DOWN:
                    if(editor.file.scroll_down() == -1) {
                        // retrieve the line after from server
                        server.send(to_string(file_contents.back().linenum + 1),
                                    C_PUSH_LINE_BACK);
                        running = S_WAITING_MODE;
                        while(running == S_WAITING_MODE)
                            ;
                        editor.file.refresh_file_content(-1);
                    }
                    break;
                case KEY_LEFT:
                    editor.file.scroll_left();
                    break;
                case KEY_RIGHT:
                    editor.file.scroll_right();
                    break;
                case KEY_BACKSPACE:
                case KEY_DC:
                case KEY_DELETE:
                case '\b':
                    editor.file.delchar();
                    server.send(to_string(editor.file.get_row()),
                                C_UPDATE_LINE_CONTENT);
                    server.send(editor.file.get_currline());
                    break;
            }
        }
    }
}


void print_welcome_screen() {
    int y = 0;
    if(max_row > welcome_screen.size() + 9 &&
       max_col > welcome_screen.front().size() + 2) {
        // if terminal is large enough to print that character image
        attron(COLOR_PAIR(2));
        for(const string& s : welcome_screen) {
            mvprintw(y++,
                     (max_col - welcome_screen.front().size() + 1) / 2,
                     "%s",
                     s.c_str());
        }
        attroff(COLOR_PAIR(2));
        attron(COLOR_PAIR(1));
        ++y;
    }

    mvprintw(y, max_col / 2 - 11, "Welcome to Hermes.");
    y += 2;

    if(!server.isconnected()) {
        int ip_y, ip_x, port_y, port_x;
        mvprintw(y++, max_col / 2 - 11, "Server ip: ");
        getyx(stdscr, ip_y, ip_x);
        mvprintw(y, max_col / 2 - 11, "Port number: ");
        getyx(stdscr, port_y, port_x);

        y += 2;
        mvprintw(y, max_col / 2 - 11, "Press CTRL+Q to quit.");
        move(ip_y, ip_x);  // ready to receive ip addrress
        string temp;
        wgetline(stdscr, temp);
        server.set_ip(temp);
        move(port_y, port_x);
        wgetline(stdscr, temp);
        server.set_port(temp);

        while(!server.connect()) {  // if connection fails
            move(max_row - 1, 0);
            clrtoeol();
            attron(COLOR_PAIR(3));  // print error message
            if(server.get_port() == "" || server.get_ip() == "")
                mvprintw(max_row - 1,
                         max_col / 2 - 20,
                         "Invalid ip/port number, please try again.");
            else
                mvprintw(max_row - 1,
                         max_col / 2 - 20,
                         "Connection to %s fails, please try again.",
                         server.get_ip().c_str());
            attroff(COLOR_PAIR(3));  // end printing error message

            move(port_y, port_x);
            clrtoeol();
            move(ip_y, ip_x);
            clrtoeol();
            refresh();
            wgetline(stdscr, temp);
            server.set_ip(temp);
            move(port_y, port_x);
            wgetline(stdscr, temp);
            server.set_port(temp);
        }

        move(y, 0);
        clrtoeol();
        mvprintw(y,
                 max_col / 2 - 17,
                 "Successfully connects to %s",
                 server.get_ip().c_str());
    } else {
        mvprintw(
            y++, max_col / 2 - 11, "Connected to: %s", server.get_ip().c_str());
        mvprintw(y++,
                 max_col / 2 - 11,
                 "Port number: %s",
                 server.get_port().c_str());
    }

    refresh();
}


bool wgetline(WINDOW* w, string& s, size_t n) {
    s.clear();
    int orig_y, orig_x;
    getyx(stdscr, orig_y, orig_x);
    int curr;  // current character to read
    while(!n || s.size() != n) {
        curr = wgetch(w);
        if(std::isprint(curr)) {
            ++orig_x;
            if(orig_x <= max_col) {
                waddch(w, curr);
                wrefresh(w);
            }

            s.push_back(curr);

        } else if(!s.empty() && (curr == KEY_BACKSPACE || curr == KEY_DC ||
                                 curr == KEY_DELETE || curr == '\b')) {
            --orig_x;
            if(orig_x <= max_col) {
                mvwdelch(w, orig_y, orig_x);
                wrefresh(w);
            }
            s.pop_back();

        } else if(curr == KEY_ENTER || curr == '\n' || curr == KEY_DOWN ||
                  curr == KEY_UP) {
            return true;
        } else if(curr == ERR) {
            if(s.empty())
                return false;
            return true;
        } else if(curr == KEY_CTRL_Q) {
            endwin();
            running = 0;
            std::exit(0);
            // return false;
        }
    }
    return true;
}

void segfault_handler(int sig) {
    // so that ncurses won't mess up our screen
    endwin();
}