#include "window.h"
#include <ncurses.h>
#include <string>
#include <vector>
using std::vector;
using std::string;

bool Window::init(int h, int w, int starty, int startx) {
    if(is_init)
        return false;
    win     = newwin(h, w, starty, startx);
    max_row = h;
    max_col = w;
    is_init = true;
    keypad(win, true);
    return true;
}

void Window::printline(const string& line, int row, int col) {
    if(!is_init)
        return;
    wmove(win, row, 0);
    wclrtoeol(win);
    wmove(win, row, col);
    waddnstr(win, line.c_str(), max_col);
    wrefresh(win);
}

void Window::clear() {
    wclear(win);
    wrefresh(win);
}

void StatusBar::print_filename(const string& file_name) {
    if(!is_init)
        return;
    wattron(win, A_REVERSE);  // print in reverse color
    wmove(win, 0, 0);
    wclrtoeol(win);
    waddch(win, ' ');
    waddnstr(win, file_name.c_str(), max_col);
    for(int i = file_name.size() + 1; i < max_col; ++i)
        waddch(win, ' ');
    wrefresh(win);
    wattroff(win, A_REVERSE);
    filename = file_name;
}

void StatusBar::print_status(const string& status) {
    if(!is_init)
        return;
    Window::printline(status, 1, 1);
}

void FileList::print_filelist(const vector<string>& file_list, int sel) {
    if(sel == -1)
        sel        = selected;
    size_t maxline = file_list.size() < max_row ? file_list.size() : max_row;
    if(!is_init || sel > maxline)
        return;


    curs_set(0);
    werase(win);
    for(size_t i = 0; i < maxline; ++i) {
        if(i != sel)
            mvwaddnstr(win, i, 0, file_list[i].c_str(), max_col);
    }

    // default is to select the first item
    wattron(win, A_REVERSE);
    mvwaddnstr(win, sel, 0, file_list[sel].c_str(), max_col);
    wattroff(win, A_REVERSE);
    selected = sel;

    wrefresh(win);
}

void FileList::scroll_up(const vector<string>& file_list) {
    if(selected == 0)
        return;
    print_filelist(file_list, --selected);
}

void FileList::scroll_down(const vector<string>& file_list) {
    if(selected >= file_list.size() - 1)
        return;
    print_filelist(file_list, ++selected);
}