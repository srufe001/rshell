FLAGS=-Wall -Werror -ansi -pedantic# -std=c++11

rshell : all

all : src/main.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/main.cpp -o bin/rshell

valgrind : src/main.cpp
	-@mkdir -p bin
	g++ $(FLAGS) -g src/main.cpp -o bin/rshell

run : all
	bin/rshell
