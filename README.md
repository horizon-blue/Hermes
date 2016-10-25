mKilo
===

### Compile

```bash
# compile release version
$ make server
$ make client

# compile debug version
$ make server-debug
$ make client-debug
```

### Usage

Run server

```bash
$ ./server [port]
```

Test echo server

```bash
$ telnet [ip] [port]
```

### Useful commands

* exit

    Close connection.

    ```
    exit
    ```

### File

```
├── LICENSE                 # License file
├── Makefile                # Makefile
├── README.md
├── TODO
├── client.c                # Client main file
├── client.h
├── deps
│   ├── api.c               # API used and parsing functions
│   ├── api.h
│   ├── configfile.c        # Read configure file
│   ├── configfile.h
│   ├── error.c             # Global error code and handler
│   ├── error.h
│   ├── socket.c            # Pack system socket
│   ├── socket.h
│   ├── util.c              # Useful utilities
│   └── util.h
├── flow.pdf                # System flow chart
├── kilo.c
├── server.c                # Server main file
├── server.h
└── tags                    # ctags
```

### Contributor

* [jamsman94](https://github.com/jamsman94)
* [Horizon-Blue](https://github.com/Horizon-Blue)
* [Rijn](https://github.com/rijn)

### Inspired by Kilo

[https://github.com/antirez/kilo](https://github.com/antirez/kilo)
