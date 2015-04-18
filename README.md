# rshell
rshell is a simple shell created for a class at school

## Installation ##

Run the folling commands in the location in which you would like to install
rshell

`$ git clone https://github.com/srufe001/rshell.git

$ cd rshell

$ make`

You can now run rshell by typing `bin/rshell` into the command prompt

## Features ##

### Run any command: ###

> `$ pwd`
> `$ ls -a`
> `$ ./my_executable`

### built-in commands ###

Type `exit` to exit rshell

> `$ exit`

### Connectors ###

rshell supports the connectors `&&`, `||`, and `;`.
`&&` executes the following command only if the previous command ran
successfully,
`||` executes the following command only if the previous command ran unsuccessfully, 
and `;` runs the following command regardless of what the previous command returned.

> `$ pwd && ls                      # this will execute pwd and ls`
> `$ false || echo Hello World      # this print Hello World`
> `$ mkdir myDir ; ls               # this will run both commands, one after another`

No command is considered a success, and a command that does not exist is
considered a failure:

> `$ && echo Hi                     # this will print Hi`
> `$ &&; echo Hi                    # this will also print Hi`
> `$ notacommand && echo Hi         # this will __not__ print Hi`

### Comments ###

Any characters following a `#` are ignored by the shell

> `$ echo this will be printed # but this will not`

## Errors ##

If a command fails to execute (the fork() syscall fails), rshell will cancel the execution of the rest of
the command, print an error message, and print another prompt.
if the wait() syscall fails, rshell will exit.

## Bugs and Issues ##

* built-in commands like `cd` are not yet implemented
* other popular features of shells like bash (pipes, input/output redirection, quoted string) are not implemented in rshell 
* rshell does not print very helpful error messages when the user makes a syntax error. 
