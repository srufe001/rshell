FLAGS=-Wall -Werror -ansi -pedantic

rshell : all

all : src/main.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/main.cpp -o bin/rshell

run : all
	bin/rshell
