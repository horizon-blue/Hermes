Hermes
===

A multiuser text editor written in C++ using ncurses library.

## Compile

```bash
$ make

# or, to compile server and client separately
$ make server
$ make client
```

To compile in debug mode (which prints out what is sent)
```bash
$ make debug

# or, to compile server and client in debug mode separately
$ make server-debug
$ make client-debug
```

## Usage
| Function                        | Command  |
| ------------------------------- |----------|
| Switch to editing/browsing mode | Ctrl + O |
| Quit (client) editor            | Ctrl + Q |
| Close server                    | Ctrl + C |

## File
The diagram is generated using [tree](https://en.wikipedia.org/wiki/Tree_(Unix))

```bash
├── client.cpp          # codes for client
├── client.h
├── deps                # Class and helper functions
│   ├── editor.cpp      # Client's editor class
│   ├── editor.h
│   ├── socket.cpp      # a wrapper class for C socket
│   ├── socket.h
│   ├── util.cpp        # Utility functions, encoding, decoding, etc.
│   ├── util.h
│   ├── window.cpp      # a wrapper class for curses WINDOW
│   └── window.h
├── diagram             # Explainatory diagrams
│   ├── flow.graffle
│   ├── flow.pdf
│   ├── server.graffle
│   └── server.pdf
├── _files              # Testing files, feel free to add your own!
│   └── ...
├── Makefile
├── _previous           # Debirs of first attempt
│   └── ...
├── README.md
├── server.cpp          # codes for server
├── server.h
└── test.cpp            # testing program that is used during development stage
```
