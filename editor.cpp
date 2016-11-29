#include "editor.h"
#include "window.h"

bool Editor::init(int maxrow, int maxcol) {
    if(is_init)
        return false;
    dir.init(maxrow - 2, maxcol, 0, 0);
    file.init(maxrow - 2, maxcol, 0, 0);
    status.init(maxrow, maxcol);
    is_init = true;
    return true;
}

void Editor::switch_mode(int mode) {
    if(mode == -1)
        mode = !current_mode;  // the other mode

    // destination mode is same as current mode
    // no change needs to be made
    if(mode == current_mode)
        return;
    if(mode == 1) {  // switch to file mode
        dir.clear();
        curs_set(1);
        // more to do here
    } else {
        file.clear();
        curs_set(0);  // hide curser
        // more to do here
    }
    current_mode = mode;
}