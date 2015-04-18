FLAGS=-Wall -Werror -ansi -pedantic# -std=c++11

all : rshell

rshell : src/main.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/main.cpp -o bin/rshell

run : all
	bin/rshell
