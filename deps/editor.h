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

    // mode 0 = directory mode
    // mode 1 = file mode
    // mode -1 = blindly switch to other mode
    void switch_mode(int mode = -1);

    FileList dir;      // directory panel
    Window file;       // file contents
    StatusBar status;  // status_bar


private:
    bool is_init     = false;
    int current_mode = 0;
};

#endif