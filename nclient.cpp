// Xiaoyan Wang 11/26/2016
// A new text editor written from starch
// Based on C++ && ncurses
#include <ncurses.h>
#include <sys/socket.h>
#include <sys/types.h>
// #include <cstdio>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "nclient.h"

using std::vector;
using std::string;
using std::mutex;
using std::thread;
using std::getchar;

int max_row, max_col;

int main() {
    // --------------- init --------------------
    initscr();             // setup ncurses screen
    raw();                 // enable raw mode so we can capture ctrl+c, etc.
    keypad(stdscr, true);  // to capture arrow key, etc.
    noecho();              // so that escape characters won't be printed
    getmaxyx(stdscr, max_row, max_col);  // get max windows size
    // -------------- done init ----------------

    print_welcome_screen();

    for(char c = getch(); c != KEY_CTRL_C; c = getch()) {  // experiment
        printw("%c", c);
        refresh();
    }
    // getch();
    endwin();  // end curse mode and restore screen

    return 0;
}

void print_welcome_screen(bool isconnected) {
    for(int i = 1; i < 14; ++i) {
        mvprintw(
            max_row / 2 - 12 + i, max_col / 2 - 34, "%s", welcome_screen[i]);
    }
    mvprintw(max_row / 2 + 3, max_col / 2 - 10, "Welcome to mKilo.\n");
    if(!isconnected) {
        mvprintw(max_row / 2 + 5, max_col / 2 - 10, "Server ip: ");
    } else {
        mvprintw(max_row / 2 + 5, max_col / 2 - 10, "Connected to: ");
    }

    refresh();
}
