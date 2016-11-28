#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "window.h"

class Editor {
public:
    Editor() = default;
    // disable copy constructor and assignment operator
    Editor(const Editor& e) = delete;
    Editor& operator=(const Editor& e) = delete;

    bool init(int maxrow, int maxcol);  // initialized windows

    // access functions
    bool isinit() const { return is_init; }


private:
    bool is_init = false;
    Window dir;     // directory panel
    Window file;    // file contents
    Window status;  // status_bar
};

#endif