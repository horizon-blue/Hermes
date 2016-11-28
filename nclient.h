#ifndef __NCLIENT_H__
#define __NCLIENT_H__

#include <ncurses.h>
#include <string>
#include "nutil.h"

#define KEY_CTRL_C 3
#define KEY_CTRL_G 7
#define KEY_CTRL_Q 17
#define KEY_DELETE 127

using std::string;

char welcome_screen[14][70] = {
    "                                                             :     \n",
    "                        G:                                  t#,    \n",
    "                        E#,    :  t               i        ;##W.   \n",
    "            ..       :  E#t  .GE  Ej             LE       :#L:WE   \n",
    "           ,W,     .Et  E#t j#K;  E#,           L#E      .KG  ,#D  \n",
    "          t##,    ,W#t  E#GK#f    E#t          G#W.      EE    ;#f \n",
    "         L###,   j###t  E##D.     E#t         D#K.      f#.     t#i\n",
    "       .E#j##,  G#fE#t  E##Wi     E#t        E#K.       :#G     GK \n",
    "      ;WW; ##,:K#i E#t  E#jL#D:   E#t      .E#E.         ;#L   LW. \n",
    "     j#E.  ##f#W,  E#t  E#t ,K#j  E#t     .K#E            t#f f#:  \n",
    "   .D#L    ###K:   E#t  E#t   jD  E#t    .K#D              f#D#;   \n",
    "  :K#t     ##D.    E#t  j#t       E#t   .W#G                G#t    \n",
    "  ...      #G      ..    ,;       E#t  :W##########Wt        t     \n",
    "           j                      ,;.  :,,,,,,,,,,,,,.             \n"};

void print_welcome_screen();
void init_colors();
bool wgetline(WINDOW* w, string& s, size_t n = 0);
void message_handler();  // TODO
void init_editor();      // TODO
#endif
