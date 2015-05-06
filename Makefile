FLAGS=-Wall -Werror -ansi -pedantic# -std=c++11

all : rshell ls cp rm mv

rshell : src/main.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/main.cpp -o bin/rshell

cp : src/cp.cpp src/Timer.h
	-@mkdir -p bin
	g++ $(FLAGS) src/cp.cpp -o bin/cp

ls : src/ls.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/ls.cpp -o bin/ls

rm : src/rm.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/rm.cpp -o bin/rm

mv : src/mv.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/mv.cpp -o bin/mv

run : all
	bin/rshell
