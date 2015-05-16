#!/bin/bash

cmds="ls  | grep bin
echo heooo | sed s_oo_ll_
exit"
echo "$cmds" | bin/rshell

#show >, <, >> overriding |.

#show that the last command's truth value is taken

#show 2 pipes ("| |") failing, pipe in front "| command", pipe behind, "command |"

#2 redirects to the same file, shoudl this work?

# show stuff after redirection such as "cat < Makefile *-b*"

# show redirection on things you don't have read/write access to?

# show rediection at front of line "2> /dev/null ls"

# incomplete forms of > >> etc "echo hello >"

# show multiple redirections to same file

# show something like "cat <<< "hello" > hi.txt" show that " works properly

# show test case with " that doesnt end. or maybe just ignore ", it wasn't
# required, was it? I'm not sure if it is
