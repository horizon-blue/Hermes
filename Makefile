.PHONY: all nclient nserver debug nserver-debug nclient-debug linecount lc
all: nclient-release nserver-release echo-done
debug: nclient-debug nserver-debug echo-done
nclient: nclient-release echo-done
nserver: nserver-release echo-done
lc: linecount

linecount:
	@cloc . --exclude-dir=./_previous,./_files,./diagram

echo-done:
	@echo "done."

nclient-release:
	g++ -std=c++11 nclient.cpp window.cpp editor.cpp nutil.cpp nsocket.cpp -o nclient -lncurses -lpthread $(WARNINGS)

nserver-release:
	g++ -std=c++11 nserver.cpp nutil.cpp nsocket.cpp -o nserver -lncurses -lpthread $(WARNINGS)

nclient-debug:
	g++ -std=c++11 nclient.cpp window.cpp editor.cpp nutil.cpp nsocket.cpp -o nclient-debug -lncurses -lpthread -DDEBUG $(WARNINGS)

nserver-debug:
	g++ -std=c++11 nserver.cpp nutil.cpp nsocket.cpp -o nserver-debug -lncurses -lpthread -DDEBUG $(WARNINGS)

.PHONY: test
test:
	g++ -std=c++11 test.cpp nutil.cpp -o test

.PHONY: clean
clean:
	rm -f nclient nserver nclient-debug nserver-debug