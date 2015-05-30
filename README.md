# rshell
rshell is a simple shell created for a class at school

## Installation ##

Run the folling commands in the location in which you would like to install
rshell

```
$ git clone https://github.com/srufe001/rshell.git

$ cd rshell

$ make
```

You can now run rshell by typing `bin/rshell` into the command prompt

## Features ##

### Run any command: ###

```
$ pwd

$ ls -a

$ ./my_executable
```

### built-in commands ###

Type `exit` to exit rshell

```
$ exit
```

type `cd` to change directory
```
# cd dir       # changes directory to "dir"
# cd -         # changes directory to the last directory you were in
# cd           # changes directory to your home directory
```

### Connectors ###

rshell supports the connectors `&&`, `||`, and `;`.
`&&` executes the following command only if the previous command ran
successfully,
`||` executes the following command only if the previous command ran unsuccessfully, 
and `;` runs the following command regardless of what the previous command returned.

```
$ pwd && ls                      # this will execute pwd and ls

$ false || echo Hello World      # this print Hello World

$ mkdir myDir ; ls               # this will run both commands, one after another
```

No command is considered a success, and a command that does not exist is
considered a failure:

```
$ && echo Hi                     # this will print Hi

$ ;; echo Hi                     # this will also print Hi

$ notacommand && echo Hi         # this will NOT print Hi
```

### Comments ###

Any characters following a `#` are ignored by the shell

```
$ echo this will be printed # but this will not
```

### Redirection ###

rshell supports the following connectors: pipe (`|`), redirection to file (`>`),
append to file (`>>`), read from file (`<`), and read from string (`<<<`). Some
examples:
```
$ echo hello | cat            # this will print "hello"
$ echo hello > file.txt
$ echo world >> file.txt      # file.txt now contains "hello" and "world"
$ cat <<< "hello world"       # this will print "hello world"
$ cat <<< hello               # this will print "hello"
```
You can specify the file descriptor to redirect when using `>` or `>>`:
```
$ echo hello 2> file.txt      # redirects stderr
```

### Signals ###

rshell supports `^C` (SIGINT) and `^Z` (SIGTSTP).
After suspending a process with `^Z`, the `fg` command can be used to resume the process, or `bg` can be used to resume the process in the background.

## Errors ##

If a command fails to execute (the fork() syscall fails), rshell will cancel the execution of the rest of
the command, print an error message, and print another prompt.
if the wait() syscall fails, rshell will exit.

## Bugs and Issues ##

* built-in commands like `cd` are not yet implemented
* other popular features of shells like bash (pipes, input/output redirection, quoted string) are not implemented in rshell 
* rshell does not print very helpful error messages when the user makes a syntax error. 
