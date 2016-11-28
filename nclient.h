#ifndef __NCLIENT_H__
#define __NCLIENT_H__

#include <ncurses.h>
#include <string>

#define KEY_CTRL_C 3
#define KEY_CTRL_G 7
#define KEY_CTRL_Q 17
#define KEY_DELETE 127

using std::string;

class Server {
public:
    Server() = default;
    Server(const string& _ip, const string& _port) : ip(_ip), port(_port) {}
    Server(const int& _s, const string& _ip, const string& _port)
        : socket(_s), ip(_ip), port(_port) {}
    ~Server() {
        if(is_connected)
            disconnect();
    }
    // access functions
    const string& get_ip() const { return ip; }
    const string& get_port() const { return port; }
    int get_socket() const { return socket; }
    bool isconnected() const { return is_connected; }

    // modification functions
    void set_socket(const int& _s) { socket = _s; }
    void set_ip(const string& _ip) { ip = _ip; }
    void set_port(const string& _port) { port = _port; }

    // connection manipulation
    bool connect();
    bool disconnect();

    // could be used directly as socket
    operator int() const { return socket; }

private:
    int socket;
    string ip;
    string port;
    bool is_connected = false;
    sockaddr_in info;
};


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
