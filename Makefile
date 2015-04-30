FLAGS=-Wall -Werror -ansi -pedantic# -std=c++11

all : rshell ls

rshell : src/main.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/main.cpp -o bin/rshell

ls : src/ls.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/ls.cpp -o bin/ls

run : all
	bin/rshell
