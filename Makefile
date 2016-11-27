# ▄▄▄▄▄  ▄▄▄▄▄    ▄▄▄  ▄▄   ▄
# █   ▀█   █        █  █▀▄  █
# █▄▄▄▄▀   █        █  █ █▄ █
# █   ▀▄   █        █  █  █ █
# █    ▀ ▄▄█▄▄  ▀▄▄▄▀  █   ██
#
# Makefile for Kilo
# https://github.com/Horizon-Blue/kilo

EXENAME = kilo

SHELL = /bin/bash

ROOT_DIR=$(shell pwd)

CXX = gcc
LD = gcc
OBJS_DEP = api.o socket.o util.o error.o configfile.o queue.o
VPATH = ./deps
OBJS_SERVER = server.o
OBJS_CLIENT = client.o
OBJS_TEST = test.o
OBJS_DIR = .objs
OPTIMIZE = off
INCLUDES = -I./includes/ -I$(OBJS_DIR)/ -Ideps/
WARNINGS = -pedantic -Wall -Werror -Wfatal-errors -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = $(INCLUDES) -std=c99 -lpthread $(WARNINGS)
CXXFLAGS = $(INCLUDES) -std=c99 -MMD -MP $(WARNINGS)
-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

ifeq ($(strip $(OPTIMIZE)),on)
CXXFLAGS += -O2 -DOPTIMIZE
else ifeq ($(strip $(OPTIMIZE)),off)
CXXFLAGS += -g -O0
else
$(warning Invalid value specified for OPTIMIZE. Should be on or off)
CXXFLAGS += -g -O0
endif

.PHONY: all server client test nclient
all: clean pre-compile server-release client echo-done
server: clean pre-compile server-release echo-done
client: clean pre-compile client-release echo-done
nclient: nclient-release echo-done
test: clean pre-compile test-debug echo-done
	clear
	@./test

pre-compile: echo-compile $(OBJS_DIR)

echo-compile:
	@echo "compiling..."

echo-done:
	@echo -e "done."

$(OBJS_DIR)/%.o: %.c
	@echo -e " cc\t$<"
	@$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJS_DIR)/%-debug.o: %.c
	@echo -e " cc\t$<"
	@$(CXX) -c $(CXXFLAGS) -DDEBUG $< -o $@

server-release: $(OBJS_DEP:%.o=$(OBJS_DIR)/%.o) $(OBJS_SERVER:%.o=$(OBJS_DIR)/%.o)
	@echo -e " ld\t$@"
	@$(LD) $^ $(LDFLAGS) -o server

server-debug: $(OBJS_DEP:%.o=$(OBJS_DIR)/%-debug.o) $(OBJS_SERVER:%.o=$(OBJS_DIR)/%-debug.o)
	@echo -e " ld\t$@"
	@$(LD) $^ $(LDFLAGS) -o server

client-release: $(OBJS_DEP:%.o=$(OBJS_DIR)/%.o) $(OBJS_CLIENT:%.o=$(OBJS_DIR)/%.o)
	@echo -e " ld\t$@"
	@$(LD) $^ $(LDFLAGS) -o client

client-debug: $(OBJS_DEP:%.o=$(OBJS_DIR)/%-debug.o) $(OBJS_CLIENT:%.o=$(OBJS_DIR)/%-debug.o)
	@echo -e " ld\t$@"
	@$(LD) $^ $(LDFLAGS) -o client

nclient-release:
	g++ -std=c++11 nclient.cpp -o nclient -lncurses -lpthread $(WARNINGS)

test-debug: $(OBJS_DEP:%.o=$(OBJS_DIR)/%-debug.o) $(OBJS_TEST:%.o=$(OBJS_DIR)/%-debug.o)
	@echo -e " ld\t$@"
	@$(LD) $^ $(LDFLAGS) -o test

.PHONY: kilo
kilo: kilo.c
	$(CC) -o kilo kilo.c -Wall -W -pedantic -std=c99

.PHONY: clean
clean:
	@rm -f $(wildcard *.d) $(wildcard *.o) $(wildcard *.cgo) $(wildcard *.cga)
	@rm -f server client kilo $(CCMONAD) $(IDFILE)
	@rm -rf .objs
