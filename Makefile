FLAGS="-Wall -Werror -ansi -pedantic"

rshell : main.cpp
	-mkdir bin
	g++ $(FLAGS) main.cpp -o bin/rshell

all : main.cpp
	-mkdir bin
	g++ $(FLAGS) main.cpp -o bin/rshell
