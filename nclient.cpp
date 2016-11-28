// Xiaoyan Wang 11/26/2016
// A new text editor written from starch
// Based on C++ && ncurses
#include <ncurses.h>
// #include <cstdio>
#include <unistd.h>
#include <cctype>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "editor.h"
#include "nclient.h"
#include "nsocket.h"
#include "nutil.h"
#include "window.h"

using std::vector;
using std::string;
using std::mutex;
// using std::thread;
// using std::getchar;

int max_row, max_col;
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

    // -------------- done init ----------------

    // ---------- establish connection ---------
    print_welcome_screen();  // let user enter ip and port
    // ---------- connection established -------

    // ---------- init editor ------------------

    // use multithread to handle message
    std::thread handler_thread(message_handler);
    init_editor();


    for(char c = getch(); c != KEY_CTRL_Q; c = getch()) {
        // printw("%c", c);
        ;
        // refresh();
    }
    // getch();

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
    server.send("*A@QkFTRQ==`Lw==* ");
    sleep(3);
}

void init_editor() {
    int y, x;
    getyx(stdscr, y, x);
    (void)x;  // silence the warning
    mvprintw(y + 1, max_col / 2 - 11, "Retrieving file list...");
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

        } else if(!s.empty() &&
                  (curr == KEY_BACKSPACE || curr == KEY_DC ||
                   curr == KEY_DELETE || curr == '\b' || curr == KEY_CTRL_G)) {
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
            std::exit(0);
            // return false;
        }
    }
    return true;
}
