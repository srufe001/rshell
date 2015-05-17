#!/bin/bash

cmds="\
##### testing pipes. myecho is a program who's 1st argument is the fd to output to, the other arguments are printed like in echo
bin/myecho 1 hello | cat # show that pipes pass on stdout
bin/myecho 2 hello | cat # show that pipes pass on stdout
bin/myecho 1 heooo | sed s_oo_ll_ | cat # show multiple pipes
| bin/myecho 1 hello # show error message with pipe at the beginning
bin/myecho 1 hello | # show error message with pipe at the end
bin/myecho 1 hello | | cat # show error message with 2 consecutive pipes
bin/myecho 1 hello|cat # show that spacing is not an issue
exit | cat # show built-in command working despite pipe\
"
echo "$cmds" | bin/rshell
cmds="\
bin/myecho 1 hello | exit | cat <<< \"hello\" # show built-in command working after a pipe\
"
echo "$cmds" | bin/rshell
cmds="\
##### testing >
bin/myecho 1 hello sam > bla.txt # show basic use, and that > can create a file
cat bla.txt
bin/myecho 2 hello mike > bla.txt # show that stderr does not redirect
cat bla.txt
> bla.txt bin/myecho 1 hello busra # show redirection at the start of a command
cat bla.txt
bin/myecho 1 hello > bla.txt world # show arguments after redirection
cat bla.txt
bin/myecho 1 hello rshell > bla.txt > bla2.txt # show redirection to 2 files. the rightmost one takes precedence
cat bla.txt
cat bla2.txt
bin/myecho 1 hello cs100>bla.txt # show that spacing is not an issue
cat bla.txt
bin/myecho hello world > bla.txt | cat # show that > and | can be used together
cat bla.txt
##### testing >> 
rm bla.txt
bin/myecho 1 hello sam >> bla.txt # show basic use, and that >> can create a file
cat bla.txt
bin/myecho 2 hello mike >> bla.txt # show that stderr does not redirect
cat bla.txt
>> bla.txt bin/myecho 1 hello busra # show redirection at the start of a command
cat bla.txt
bin/myecho 1 hello >> bla.txt world # show arguments after redirection
cat bla.txt
bin/myecho 1 hello rshell >> bla.txt >> bla2.txt # show redirection to 2 files. the rightmost one takes precedence
cat bla.txt
cat bla2.txt
bin/myecho 1 hello cs100>>bla.txt # show that spacing is not an issue
cat bla.txt
bin/myecho 1 hello cs100 >>> bla.txt # rshell will think the 3rd > is the file to append to
cat bla.txt
bin/myecho hello world >> bla.txt | cat # show that >> and | can be used together
cat bla.txt
bin/myecho 1 hello bla > bla.txt | bin/myecho 1 hello sam >> bla.txt # this should have strange results. I don't support this case
cat bla.txt
##### test <
bin/myecho 1 > bla.txt hello world # bla.txt will be used to help test <
cat < bla.txt # test basic use
cat<bla.txt # show that spacing is not an issue
cat < notafile # show error message when the file doesnt exist
cat < bla.txt | cat show < working with |
cat < bla.txt > bla.txt # show > working with <
cat bla.txt
cat > bla.txt < bla.txt # show > working with < in a different order
cat bla.txt
cat < bla.txt >> bla.txt # show >> working with <
cat bla.txt
cat >> bla.txt < bla.txt # show >> working with < in a different order
cat bla.txt
bin/myecho 1 hello cs100 > bla2.txt # this will be used to help test
cat < bla.txt < bla2.txt # show 2 redirections. the rightmost one will take precedence 
< bla.txt cat # show redirection at start of command
cat < bla.txt -b # show arguments after redirection
cat -b < bla.txt # show < after argument
cat<bla.txt # show that spacing is not an issue
##### test quoted strings with \"
bin/myecho 1 \"hello ### || && ; | > >> <<< < sam\" # show that quote doesn't proccess spaces or other special symbols specially
\"bin/myecho\" 1 \"hello sam\" # show that quote can be used for executables too
# show that the end quote is not necessary
bin/myecho 1 \"hello sam
bin/myecho 1 I\" say hello \"sam # show quotations inside of an argument
##### test extra credit 1: <<< operator
cat <<< hello # show basic use without 
cat <<< \" hello sam \" # show basic use with quoted string
cat < bla.txt <<< cs100 # show that rightmost redirection takes precedence
cat <<< cs100 < bla.txt # show that rightmost redirection takes precedence
<<< \"cs100 is my favorite class so far. except maybe cs61\" cat # show <<< at start of command
cat <<< cs100 > bla.txt # show <<< in use with a redirector that redirects stdout
cat bla.txt
cat<<<\"hi sam\" # show that spacing is not an issue
##### test extra credit 2:
bin/myecho 1 hello 1> bla.txt # show redirection to stdout
cat bla.txt
bin/myecho 2 hello 1> bla.txt # show that the wrong file descriptor does not redirect
cat bla.txt
bin/myecho 2 hello 2> bla.txt # show redirection of stderr
cat bla.txt
bin/myecho 3 hello 3> bla.txt # show redirection of 3
cat bla.txt
bin/myecho 450 hello 450> bla.txt # show redirection of some random file descriptor
cat bla.txt
bin/myecho 1 hello2> bla.txt # show how > does not grab the previous argument as the file descriptor number if its not a number
cat bla.txt
bin/myecho 4 hello 4>> bla.txt # show it working with >>
cat bla.txt
bin/myecho 1 hello 0> bla.txt # redirecting stdin
cat bla.txt
##### show many different redirections working together
bin/myecho 1 give me an F plus | sed s_F_A_ | cat ; cat <<< \"cs100 sucks\" | sed s_sucks_rocks_ > bla.txt && bin/myecho 3 best class 2015 2>> bla.txt
cat bla.txt
exit\
"
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
#show echo bla2> bla.txt does not redirect
