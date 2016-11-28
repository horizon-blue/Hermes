#include "editor.h"
#include "window.h"

bool Editor::init(int maxrow, int maxcol) {
    if(is_init)
        return false;
    dir.init(maxrow - 2, maxcol, 0, 0);
    file.init(maxrow - 2, maxcol, 0, 0);
    status.init(2, maxcol, maxrow - 2, 0);
    is_init = true;
    return true;
}