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
    for(size_t i = file_name.size() + 1; i < max_col; ++i)
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

void FileList::print_filelist(const vector<string>& file_list, ssize_t sel) {
    size_t chosen;
    if(sel == -1)
        chosen = selected;
    else
        chosen     = sel;
    size_t maxline = file_list.size() < max_row ? file_list.size() : max_row;
    if(!is_init || chosen > maxline)
        return;


    curs_set(0);
    werase(win);
    for(size_t i = 0; i < maxline; ++i) {
        if(i != chosen)
            mvwaddnstr(win, i, 0, file_list[i].c_str(), max_col);
    }

    // default is to select the first item
    wattron(win, A_REVERSE);
    mvwaddnstr(win, chosen, 0, file_list[chosen].c_str(), max_col);
    wattroff(win, A_REVERSE);
    selected = chosen;

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

void FileContent::set_file_content(list<ClientLineEntry>* fc,
                                   int row,
                                   int col) {
    file_content = fc;
    currrow      = fc->begin();
    currrow_num  = row;
    while(row--)
        ++currrow;
    currcol = col;
    wmove(win, row, col);
}

list<ClientLineEntry>::iterator FileContent::get_line(int row) {
    auto ret = file_content->begin();
    while(row--)
        ++ret;
    return ret;
}

void FileContent::refresh_file_content(int row) {
    if(row != -1) {
        Window::printline(get_line(row)->s, row);
        wmove(win, currrow_num, currcol);
        wrefresh(win);
        return;
    }
    werase(win);
    wmove(win, 0, 0);
    for(const auto& l : *file_content)
        mvwaddnstr(win, ++row, 0, l.s.c_str(), max_col);
    wrefresh(win);
    currrow = std::move(get_line(currrow_num));
    if((currrow->s).size() < currcol) {
        currcol = currrow->s.size();
    }
    wmove(win, currrow_num, currcol);
}

void FileContent::refresh_currrow() {
    Window::printline(currrow->s, currrow_num);
    wmove(win, currrow_num, currcol);
    wrefresh(win);
}

int FileContent::scroll_up() {
    if(currrow_num == 0) {
        if(currrow->linenum)
            return -1;  // ask to retieve the line before
        else
            return -2;  // we are at the front of file
    }
    --currrow;  // move back a line
    if((currrow->s).size() < currcol) {
        currcol = currrow->s.size();
    }
    wmove(win, --currrow_num, currcol);
    return 1;
}

int FileContent::scroll_down() {
    if(currrow_num == max_row - 1) {
        if(currrow->linenum == num_file_lines)
            return -2;
        else
            return -1;  // ask to retieve the line after
    }
    if(++currrow == file_content->end()) {
        --currrow;
        return -2;
    }
    if((currrow->s).size() < currcol) {
        currcol = currrow->s.size();
    }
    wmove(win, ++currrow_num, currcol);
    return 1;
}

int FileContent::scroll_right() {
    if(currcol >= max_col || currcol == currrow->s.size())
        return 0;
    wmove(win, currrow_num, ++currcol);
    return 1;
}

int FileContent::scroll_left() {
    if(currcol == 0)
        return 0;
    --currcol;
    wmove(win, currrow_num, currcol);
    return 1;
}

void FileContent::insertchar(const char& c) {
    if(currcol > max_col)
        return;
    currrow->s.insert(currrow->s.begin() + currcol, c);
    refresh_currrow();
    wmove(win, currrow_num, ++currcol);
}

void FileContent::delchar() {
    if(currrow->s.empty() || currcol == 0)
        return;
    if(currrow->s.size() < currcol) {
        PERROR("delete character past end of line");
        return;
    }
    --currcol;
    currrow->s.erase(currrow->s.begin() + currcol);
    refresh_currrow();
    wmove(win, currrow_num, currcol);
}