FLAGS=-Wall -Werror -ansi -pedantic# -std=c++11

all : rshell ls cp rm mv

rshell : bin/rshell
bin/rshell : src/main.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/main.cpp -o bin/rshell

cp : bin/cp
bin/cp : src/cp.cpp src/Timer.h
	-@mkdir -p bin
	g++ $(FLAGS) src/cp.cpp -o bin/cp

ls : bin/ls
bin/ls : src/ls.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/ls.cpp -o bin/ls

rm : bin/rm
bin/rm : src/rm.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/rm.cpp -o bin/rm

mv : bin/mv
bin/mv : src/mv.cpp
	-@mkdir -p bin
	g++ $(FLAGS) src/mv.cpp -o bin/mv

run : rshell
	bin/rshell

clean :
	rm -rf bin
