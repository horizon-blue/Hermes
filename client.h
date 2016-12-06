#ifndef __NCLIENT_H__
#define __NCLIENT_H__

#include <ncurses.h>
#include <string>
#include <vector>
#include "util.h"

enum CTRL_KEY_TYPE {
    KEY_CTRL_C = 3,
    KEY_CTRL_G = 7,
    KEY_CTRL_M = 13,
    KEY_CTRL_Q = 17,
    KEY_DELETE = 127,
};

using std::string;
using std::vector;

// [11][63]
vector<string> welcome_screen = {
    "____    ____                                                  \n",
    "`MM'    `MM'                                                  \n",
    " MM      MM                                                   \n",
    " MM      MM   ____   ___  __ ___  __    __     ____     ____  \n",
    " MM      MM  6MMMMb  `MM 6MM `MM 6MMb  6MMb   6MMMMb   6MMMMb\\\n",
    " MMMMMMMMMM 6M'  `Mb  MM69 \"  MM69 `MM69 `Mb 6M'  `Mb MM'    `\n",
    " MM      MM MM    MM  MM'     MM'   MM'   MM MM    MM YM.     \n",
    " MM      MM MMMMMMMM  MM      MM    MM    MM MMMMMMMM  YMMMMb \n",
    " MM      MM MM        MM      MM    MM    MM MM            `Mb\n",
    " MM      MM YM    d9  MM      MM    MM    MM YM    d9 L    ,MM\n",
    "_MM_    _MM_ YMMMM9  _MM_    _MM_  _MM_  _MM_ YMMMM9  MYMMMM9 \n"};


void print_welcome_screen();
void init_colors();
bool wgetline(WINDOW* w, string& s, size_t n = 0);
void message_handler();  // TODO
void run_editor();       // TODO
void segfault_handler(int sig);

#endif
