#ifndef __WINDOW_H__
#define __WINDOW_H__
#include <ncurses.h>
#include <list>
#include <string>
#include <vector>
#include "util.h"
using std::vector;
using std::list;
using std::string;

class Window {
public:
    Window() = default;
    Window(int h, int w, int starty, int startx) : max_row(h), max_col(w) {
        is_init = true;
        win     = newwin(h, w, starty, startx);
    }
    // disable copy constructor and assignment operator
    Window(const Window& s) = delete;
    Window& operator=(const Window& s) = delete;

    bool init(int h, int w, int starty, int startx);

    void printline(const string& line, int row, int col = 0);
    void clear();

    WINDOW* get_window() const { return win; }
    bool isinit() const { return is_init; }

    operator WINDOW*() const { return win; }


    ~Window() {
        if(is_init)
            delwin(win);
    }

protected:
    bool is_init = false;
    WINDOW* win;  // status bar
    size_t max_row = 0;
    size_t max_col = 0;
};

class StatusBar : public Window {
public:
    StatusBar() : Window() {}
    StatusBar(int h, int w, int starty, int startx)
        : Window(h, w, starty, startx) {}
    StatusBar(int maxrow, int maxcol) : Window(2, maxcol, maxrow - 2, 0) {}

    bool init(int maxrow, int maxcol) {
        return Window::init(2, maxcol, maxrow - 2, 0);
    }

    void print_filename(const string& file_name);
    void print_status(const string& status);

    string get_filename() const { return filename; }

private:
    string filename;
};

class FileList : public Window {
public:
    void print_filelist(const vector<string>& file_list, ssize_t sel = -1);
    void scroll_up(const vector<string>& file_list);
    void scroll_down(const vector<string>& file_list);

    int get_selection() const { return selected; }

private:
    size_t selected = 0;
};

class FileContent : public Window {
public:
    int scroll_up();
    int scroll_down();
    int scroll_right();
    int scroll_left();
    void insertchar(const char& c);  // insert character at current position
    void delchar();
    void set_num_file_lines(const size_t& i) { num_file_lines = i; }
    list<ClientLineEntry>::iterator get_line(int row);
    void set_file_content(list<ClientLineEntry>* fc, int row = 0, int col = 0);
    void refresh_file_content(int row = -1);  // row = -1 -> refresh entire file
    void refresh_file_content(list<ClientLineEntry>::iterator& iter, int row);
    void refresh_file_content(const string& line, int row);
    void refresh_currrow();
    const string& get_currline() const { return currrow->s; }
    size_t get_row() const { return currrow->linenum; }

    bool isediting = false;
    vector<bool> other_status_vec;

private:
    list<ClientLineEntry>* file_content;
    list<ClientLineEntry>::iterator currrow;
    size_t currrow_num;
    size_t currcol;
    size_t num_file_lines;
};

#endif