# Makefile for Hermes Text Editor
# Xiaoyan Wang (HorizonBlue) December, 2016
OBJS_DIR = .objs
DEPS_DIR = deps
EXE_SERVER = server
EXE_CLIENT = client
EXES_ALL = $(EXE_SERVER) $(EXE_CLIENT)

# dependencies
OBJS_DEP = editor.o socket.o util.o window.o
# OBJS_SERVER = server.o $(OBJS_DEP)
# OBJS_CLIENT = client.o $(OBJS_DEP)

# set up compiler
CC = g++
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter
CFLAGS_DEBUG   = -O0 $(WARNINGS) -I$(DEPS_DIR)/ -std=c++11 -c -DDEBUG
CFLAGS_RELEASE = -O2 $(WARNINGS) -I$(DEPS_DIR)/ -std=c++11 -c

# set up linker
LD = g++
LDFLAGS = -lncurses -lpthread -I$(DEPS_DIR)/ -I$(OBJS_DIR)/ -std=c++11
-include $(OBJS_DIR)/*.d

.PHONY: all
all: release

.PHONY: release
.PHONY: debug

release: echo-compile $(EXES_ALL) echo-done
debug:   echo-compile $(EXES_ALL:%=%-debug) echo-done

.PHONY: echo-compile echo-done

echo-compile:
	@echo "compiling..."

echo-done:
	@echo "done."


$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# compiling objs
$(OBJS_DIR)/%-debug.o: $(DEPS_DIR)/%.cpp | $(OBJS_DIR)
	@echo " cc\t$<"
	@$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-release.o: $(DEPS_DIR)/%.cpp | $(OBJS_DIR)
	@echo " cc\t$<"
	@$(CC) $(CFLAGS_RELEASE) $< -o $@

# compiling exes
$(EXE_SERVER): $(OBJS_DEP:%.o=$(OBJS_DIR)/%-release.o)
	@echo " ld\t$@"
	@$(LD) server.cpp $^ $(LDFLAGS) -o $@

$(EXE_SERVER)-debug: $(OBJS_DEP:%.o=$(OBJS_DIR)/%-debug.o)
	@echo " ld\t$@"
	@$(LD) server.cpp $^ $(LDFLAGS) -o $@

$(EXE_CLIENT): $(OBJS_DEP:%.o=$(OBJS_DIR)/%-release.o)
	@echo " ld\t$@"
	@$(LD) client.cpp $^ $(LDFLAGS) -o $@

$(EXE_CLIENT)-debug: $(OBJS_DEP:%.o=$(OBJS_DIR)/%-debug.o)
	@echo " ld\t$@"
	@$(LD) client.cpp  $^ $(LDFLAGS) -o $@

# line count
.PHONY: lc linecount
lc: linecount

linecount:
	@cloc . --exclude-dir=./_previous,./_files,./diagram

.PHONY: clean
clean:
	rm -rf .objs $(EXES_ALL) $(EXES_ALL:%=%-debug)
