#define _BSD_SOURCE
#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "api.h"
#include "queue.h"
#include "socket.h"
#include "util.h"

int is_cleared;

Socket _s;

char*          server_ip;
unsigned short server_port;
pthread_t      message_handler_thread;

typedef struct dir_t {
    char*    path;
    long     size;
    uint64_t last_modified_time;

} dir_t;

struct editorConfig {
    int                  cx, cy;     /* Cursor x and y position in characters */
    int                  rowoff;     /* Offset of row displayed. */
    int                  coloff;     /* Offset of column displayed. */
    int                  screenrows; /* Number of rows that we can show */
    int                  screencols; /* Number of cols that we can show */
    int                  numrows;    /* Number of rows */
    int                  rawmode;    /* Is terminal raw mode enabled? */
    int                  dirty;      /* File modified but not saved. */
    char*                filename;   /* Currently open filename */
    char                 statusmsg[80];
    time_t               statusmsg_time;
    struct editorSyntax* syntax; /* Current syntax highlight, or NULL. */

    int    mode;
    int    numfiles;
    dir_t* directory;
};

static struct editorConfig E;

enum KEY_ACTION {
    KEY_NULL  = 0,  /* NULL */
    CTRL_C    = 3,  /* Ctrl-c */
    CTRL_D    = 4,  /* Ctrl-d */
    CTRL_F    = 6,  /* Ctrl-f */
    CTRL_H    = 8,  /* Ctrl-h */
    TAB       = 9,  /* Tab */
    CTRL_L    = 12, /* Ctrl+l */
    ENTER     = 13, /* Enter */
    CTRL_Q    = 17, /* Ctrl-q */
    CTRL_S    = 19, /* Ctrl-s */
    CTRL_U    = 21, /* Ctrl-u */
    CTRL_W    = 23,
    ESC       = 27,  /* Escape */
    BACKSPACE = 127, /* Backspace */
    /* The following are just soft codes, not really reported by the
     * terminal directly. */
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

enum WINDOW_MODE { FILE_MODE, DIR_MODE };

static void clear();

/* ======================= Low level terminal handling ====================== */

static struct termios orig_termios; /* In order to restore at exit.*/

void disableRawMode( int fd ) {
    /* Don't even check the return value as it's too late. */
    if ( E.rawmode ) {
        tcsetattr( fd, TCSAFLUSH, &orig_termios );
        E.rawmode = 0;
    }
}

/* Called at exit to avoid remaining in raw mode. */
void editorAtExit( void ) {
    disableRawMode( STDIN_FILENO );
}

/* Raw mode: 1960 magic shit. */
int enableRawMode( int fd ) {
    struct termios raw;

    if ( E.rawmode ) {
        return 0; /* Already enabled. */
    }

    if ( !isatty( STDIN_FILENO ) ) {
        goto fatal;
    }

    atexit( editorAtExit );

    if ( tcgetattr( fd, &orig_termios ) == -1 ) {
        goto fatal;
    }

    raw = orig_termios; /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~( BRKINT | ICRNL | INPCK | ISTRIP | IXON );
    /* output modes - disable post processing */
    raw.c_oflag &= ~( OPOST );
    /* control modes - set 8 bit chars */
    raw.c_cflag |= ( CS8 );
    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~( ECHO | ICANON | IEXTEN | ISIG );
    /* control chars - set return condition: min number of bytes and timer. */
    raw.c_cc[VMIN]  = 0; /* Return each byte, or zero for timeout. */
    raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */

    /* put terminal in raw mode after flushing */
    if ( tcsetattr( fd, TCSAFLUSH, &raw ) < 0 ) {
        goto fatal;
    }

    E.rawmode = 1;
    return 0;

fatal:
    errno = ENOTTY;
    return -1;
}

/* Read a key from the terminal put in raw mode, trying to handle
 * escape sequences. */
int editorReadKey( int fd ) {
    int  nread;
    char c, seq[3];

    while ( ( nread = read( fd, &c, 1 ) ) == 0 )
        ;

    if ( nread == -1 ) {
        exit( 1 );
    }

    while ( 1 ) {
        switch ( c ) {
            case ESC: /* escape sequence */

                /* If this is just an ESC, we'll timeout here. */
                if ( read( fd, seq, 1 ) == 0 ) {
                    return ESC;
                }

                if ( read( fd, seq + 1, 1 ) == 0 ) {
                    return ESC;
                }

                /* ESC [ sequences. */
                if ( seq[0] == '[' ) {
                    if ( seq[1] >= '0' && seq[1] <= '9' ) {
                        /* Extended escape, read additional byte. */
                        if ( read( fd, seq + 2, 1 ) == 0 ) {
                            return ESC;
                        }

                        if ( seq[2] == '~' ) {
                            switch ( seq[1] ) {
                                case '3':
                                    return DEL_KEY;

                                case '5':
                                    return PAGE_UP;

                                case '6':
                                    return PAGE_DOWN;
                            }
                        }
                    } else {
                        switch ( seq[1] ) {
                            case 'A':
                                return ARROW_UP;

                            case 'B':
                                return ARROW_DOWN;

                            case 'C':
                                return ARROW_RIGHT;

                            case 'D':
                                return ARROW_LEFT;

                            case 'H':
                                return HOME_KEY;

                            case 'F':
                                return END_KEY;
                        }
                    }
                }

                /* ESC O sequences. */
                else if ( seq[0] == 'O' ) {
                    switch ( seq[1] ) {
                        case 'H':
                            return HOME_KEY;

                        case 'F':
                            return END_KEY;
                    }
                }

                break;

            default:
                return c;
        }
    }
}

/* Use the ESC [6n escape sequence to query the horizontal cursor position
 * and return it. On error -1 is returned, on success the position of the
 * cursor is stored at *rows and *cols and 0 is returned. */
int getCursorPosition( int ifd, int ofd, int* rows, int* cols ) {
    char         buf[32];
    unsigned int i = 0;

    /* Report cursor location */
    if ( write( ofd, "\x1b[6n", 4 ) != 4 ) {
        return -1;
    }

    /* Read the response: ESC [ rows ; cols R */
    while ( i < sizeof( buf ) - 1 ) {
        if ( read( ifd, buf + i, 1 ) != 1 ) {
            break;
        }

        if ( buf[i] == 'R' ) {
            break;
        }

        i++;
    }

    buf[i] = '\0';

    /* Parse it. */
    if ( buf[0] != ESC || buf[1] != '[' ) {
        return -1;
    }

    if ( sscanf( buf + 2, "%d;%d", rows, cols ) != 2 ) {
        return -1;
    }

    return 0;
}

/* Try to get the number of columns in the current terminal. If the ioctl()
 * call fails the function will try to query the terminal itself.
 * Returns 0 on success, -1 on error. */
int getWindowSize( int ifd, int ofd, int* rows, int* cols ) {
    struct winsize ws;

    if ( ioctl( 1, TIOCGWINSZ, &ws ) == -1 || ws.ws_col == 0 ) {
        /* ioctl() failed. Try to query the terminal itself. */
        int orig_row, orig_col, retval;

        /* Get the initial position so we can restore it later. */
        retval = getCursorPosition( ifd, ofd, &orig_row, &orig_col );

        if ( retval == -1 ) {
            goto failed;
        }

        /* Go to right/bottom margin and get position. */
        if ( write( ofd, "\x1b[999C\x1b[999B", 12 ) != 12 ) {
            goto failed;
        }

        retval = getCursorPosition( ifd, ofd, rows, cols );

        if ( retval == -1 ) {
            goto failed;
        }

        /* Restore position. */
        char seq[32];
        snprintf( seq, 32, "\x1b[%d;%dH", orig_row, orig_col );

        if ( write( ofd, seq, strlen( seq ) ) == -1 ) {
            /* Can't recover... */
        }

        return 0;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }

failed:
    return -1;
}

/* ============================= Terminal update ============================ */

/* We define a very simple "append buffer" structure, that is an heap
 * allocated string where we can append to. This is useful in order to
 * write all the escape sequences in a buffer and flush them to the standard
 * output in a single call, to avoid flickering effects. */
struct abuf {
    char* b;
    int   len;
};

#define ABUF_INIT \
    { NULL, 0 }

void abAppend( struct abuf* ab, const char* s, int len ) {
    char* new = realloc( ab->b, ab->len + len );

    if ( new == NULL ) {
        return;
    }

    memcpy( new + ab->len, s, len );
    ab->b = new;
    ab->len += len;
}

void abFree( struct abuf* ab ) {
    free( ab->b );
}

/* This function writes the whole screen using VT100 escape characters
 * starting from the logical state of the editor in the global state 'E'. */
void editorRefreshScreen( void ) {
    int         y;
    char        buf[32];
    struct abuf ab = ABUF_INIT;

    abAppend( &ab, "\x1b[?25l", 6 ); /* Hide cursor. */
    abAppend( &ab, "\x1b[H", 3 );    /* Go home. */

    for ( y = 0; y < E.screenrows; y++ ) {
        int filerow = E.rowoff + y;

        if ( filerow >= E.numrows ) {
            if ( E.numrows == 0 && y == E.screenrows / 3 ) {
                char welcome[80];
                int  welcomelen =
                    snprintf( welcome, sizeof( welcome ), "mKilo" );
                int padding = ( E.screencols - welcomelen ) / 2;

                if ( padding ) {
                    abAppend( &ab, "~", 1 );
                    padding--;
                }

                while ( padding-- ) {
                    abAppend( &ab, " ", 1 );
                }

                abAppend( &ab, welcome, welcomelen );
            } else {
                abAppend( &ab, "~\x1b[0K\r\n", 7 );
            }

            continue;
        }

        abAppend( &ab, "\x1b[39m", 5 );
        abAppend( &ab, "\x1b[0K", 4 );
        abAppend( &ab, "\r\n", 2 );
    }

    /* Create a two rows status. First row: */
    abAppend( &ab, "\x1b[0K", 4 );
    abAppend( &ab, "\x1b[7m", 4 );
    char status[80], rstatus[80];
    int  len = snprintf( status, sizeof( status ), "%.20s - %d lines %s",
                        E.filename, E.numrows, E.dirty ? "(modified)" : "" );
    int rlen = snprintf( rstatus, sizeof( rstatus ), "%d/%d",
                         E.rowoff + E.cy + 1, E.numrows );

    if ( len > E.screencols ) {
        len = E.screencols;
    }

    abAppend( &ab, status, len );

    while ( len < E.screencols ) {
        if ( E.screencols - len == rlen ) {
            abAppend( &ab, rstatus, rlen );
            break;
        } else {
            abAppend( &ab, " ", 1 );
            len++;
        }
    }

    abAppend( &ab, "\x1b[0m\r\n", 6 );

    /* Second row depends on E.statusmsg and the status message update time. */
    abAppend( &ab, "\x1b[0K", 4 );
    int msglen = strlen( E.statusmsg );

    if ( msglen && time( NULL ) - E.statusmsg_time < 5 ) {
        abAppend( &ab, E.statusmsg,
                  msglen <= E.screencols ? msglen : E.screencols );
    }

    /* Put cursor at its current position. Note that the horizontal position
     * at which the cursor is displayed may be different compared to 'E.cx'
     * because of TABs. */
    int j;
    int cx      = 1;
    int filerow = E.rowoff + E.cy;

    snprintf( buf, sizeof( buf ), "\x1b[%d;%dH", E.cy + 1, cx );
    abAppend( &ab, buf, strlen( buf ) );
    abAppend( &ab, "\x1b[?25h", 6 ); /* Show cursor. */
    write( STDOUT_FILENO, ab.b, ab.len );
    abFree( &ab );
}

/* ========================= Editor events handling  ======================== */

/* Handle cursor position change because arrow keys were pressed. */
void editorMoveCursor( int key ) {
    int filerow = E.rowoff + E.cy;
    int filecol = E.coloff + E.cx;
    int rowlen;
}

/* Process events arriving from the standard input, which is, the user
 * is typing stuff on the terminal. */
#define KILO_QUIT_TIMES 3
void editorProcessKeypress( int fd ) {
    /* When the file is modified, requires Ctrl-q to be pressed N times
     * before actually quitting. */
    static int quit_times = KILO_QUIT_TIMES;

    int c = editorReadKey( fd );

    switch ( c ) {
        case ENTER: /* Enter */
            // editorInsertNewline();
            break;

        case CTRL_C: /* Ctrl-c */
            /* We ignore ctrl-c, it can't be so simple to lose the changes
             * to the edited file. */
            break;

        case CTRL_Q: /* Ctrl-q */
            /* Quit if the file was already saved. */
            clear();
            break;

        case CTRL_S: /* Ctrl-s */
            break;

        case CTRL_F:
            break;

        case CTRL_W:
            E.mode = E.mode == FILE_MODE ? DIR_MODE : FILE_MODE;
            break;

        case BACKSPACE: /* Backspace */
        case CTRL_H:    /* Ctrl-h */
        case DEL_KEY:
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            if ( c == PAGE_UP && E.cy != 0 ) {
                E.cy = 0;
            } else if ( c == PAGE_DOWN && E.cy != E.screenrows - 1 ) {
                E.cy = E.screenrows - 1;
            }

            {
                int times = E.screenrows;

                while ( times-- )
                    editorMoveCursor( c == PAGE_UP ? ARROW_UP : ARROW_DOWN );
            }

            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor( c );
            break;

        case CTRL_L: /* ctrl+l, clear screen */
            /* Just refresht the line as side effect. */
            break;

        case ESC:
            /* Nothing to do for ESC in this mode. */
            break;

        default:
            break;
    }

    quit_times = KILO_QUIT_TIMES; /* Reset it to the original value. */
}

/* Set an editor status message for the second line of the status, at the
 * end of the screen. */
void editorSetStatusMessage( const char* fmt, ... ) {
    va_list ap;
    va_start( ap, fmt );
    vsnprintf( E.statusmsg, sizeof( E.statusmsg ), fmt, ap );
    va_end( ap );
    E.statusmsg_time = time( NULL );
}

void initEditor( void ) {
    E.cx       = 0;
    E.cy       = 0;
    E.rowoff   = 0;
    E.coloff   = 0;
    E.numrows  = 0;
    E.dirty    = 0;
    E.filename = NULL;
    E.syntax   = NULL;

    if ( getWindowSize( STDIN_FILENO, STDOUT_FILENO, &E.screenrows,
                        &E.screencols ) == -1 ) {
        perror( "Unable to query the screen for size (columns / rows)" );
        exit( 1 );
    }

    E.screenrows -= 2; /* Get room for status bar. */

    E.mode = DIR_MODE;
}

/* ======================== Socket ======================= */

void* message_handler( void* ptr ) {
    Socket*      self   = (Socket*)ptr;
    static char* buffer = NULL;
    size_t       length;

    while ( 1 ) {
        if ( self->receive( self, &buffer, &length ) ) {
            clear();
            return NULL;
        }

#ifdef DEBUG
        fprintf( stderr, "[%s] received: %s\n", __func__, buffer );
#endif
    }

    return NULL;
}

void initialize_connection( char* ip, unsigned short port ) {
#ifdef DEBUG
    printf( "[%s] starting client\n", __func__ );
#endif

    initialize_socket( &_s );

    _s.create( &_s, ip, port, 0 );
    _s.connect( &_s );

    /* create thread */
    pthread_create( &message_handler_thread, 0, message_handler, &_s );

    return;
}

int main( int argc, char** argv ) {
    is_cleared = 0;

    printf( "Welcome to mkilo\n" );

    printf( "Server ip: " );
    server_ip = malloc( 16 );
    scanf( "%s", server_ip );

    printf( "Server port: " );
    scanf( "%hd", &server_port );

#ifdef DEBUG
    printf( "[%s] server_ip [%s]\n", __func__, server_ip );
    printf( "[%s] server_port [%hu]\n", __func__, server_port );
#endif

    initialize_connection( server_ip, server_port );

    /* initialize api parser */
    api_initialize();

    /* ask server for file list */
    api_clear();
    char*   command     = NULL;
    ssize_t command_len = 0;
    api_set_key( "BASE", "/" );
    api_generator( &command, &command_len, API_GET_REMOTE_FILE_LIST );
    _s.send( &_s, command, command_len + 1 );

    // initEditor();
    // enableRawMode ( STDIN_FILENO );
    // editorSetStatusMessage (
    //     "HELP: Ctrl-Q = quit | Ctrl-W = change windows" );

    // while ( 1 )
    // {
    //     editorRefreshScreen();
    //     editorProcessKeypress ( STDIN_FILENO );
    // }

    clear();

    return 0;
}

static void clear() {
    if ( is_cleared ) {
        return;
    }

    is_cleared = 1;

    disableRawMode( STDIN_FILENO );

    api_clear();

    if ( message_handler_thread != NULL ) {
        pthread_kill( message_handler_thread, SIGKILL );
    }

    pthread_exit( NULL );

#ifdef DEBUG
    printf( "[%s] closing client\n", __func__ );
#endif
    _s.close( &_s );

    // if(server_ip) {
    //     free ( server_ip );
    //     server_ip = NULL;
    // }

    exit( 0 );
}
