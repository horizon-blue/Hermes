#ifndef __WINDOW_H__
#define __WINDOW_H__
#include <ncurses.h>

class Window {
public:
    Window() = default;
    Window(int h, int w, int starty, int startx) {
        is_init = true;
        win     = newwin(h, w, starty, startx);
    }
    // disable copy constructor and assignment operator
    Window(const Window& s) = delete;
    Window& operator=(const Window& s) = delete;

    bool init(int h, int w, int starty, int startx);

    bool isinit() const { return is_init; }
    WINDOW* get_window() const { return win; }

    operator WINDOW*() const { return win; }

    ~Window() {
        if(is_init)
            delwin(win);
    }

private:
    bool is_init = false;
    WINDOW* win;  // status bar
};

#endif