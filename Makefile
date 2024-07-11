CFLAGS=-include default_include.hpp -std=c++17 -I./include -I./msgpack11
CXX=g++
DEPS=src/main.o src/client.o src/messages.o msgpack11/msgpack11.o

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS)

build/main: $(DEPS)
	$(CXX) -o build/main $(DEPS)
