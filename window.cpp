#include "window.h"
#include <ncurses.h>

bool Window::init(int h, int w, int starty, int startx) {
    if(is_init)
        return false;
    win     = newwin(h, w, starty, startx);
    is_init = true;
    return true;
}
