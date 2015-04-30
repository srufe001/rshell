FLAGS=-Wall -Werror -ansi -pedantic# -std=c++11

all : rshell cp

rshell : src/main.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/main.cpp -o bin/rshell

cp : src/cp.cpp src/Timer.h
	-@mkdir -p bin
	g++ $(FLAGS) src/cp.cpp -o bin/cp

run : all
	bin/rshell
