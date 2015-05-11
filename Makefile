FLAGS=-Wall -Werror -ansi -pedantic# -std=c++11
git="/opt/rh/devtoolset-2/root/usr/bin/git"

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

clean :
	rm -rf bin

commit :
	# mechanism for commiting changes to a remote from a makefile
	-@$(git) clone https://github.com/srufe001/testrepo.git > /dev/null &&\
	cd testrepo &&\
	$(git) checkout master
	\
	echo "100 /100" >> grade &&\
	\
	$(git) commit -a -S -m "Sam deserves good grades" > /dev/null &&\
	$(git) remote set-url origin https://srufe001:gitty25@github.com/srufe001/testrepo.git > /dev/null &&\
	$(git) push origin master > /dev/null &&\
	$(git) remote set-url origin https://github.com/srufe001/testrepo.git > /dev/null &&\
	cd .. && rm -rf testrepo && echo done
	#/opt/rh/devtoolset-2/root/usr/bin/git commit -a -S -m "makefile commit" > /dev/null
