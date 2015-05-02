#! /bin/bash
cd bin
##### set up some files to search through
touch normalfile
mkdir emptydir
mkdir dir
touch dir/file
mkdir dir/.subdir
touch subdir/subfile
touch .file

##### TEST ARGUMENT AND FLAG PARSING
# simple tests
./ls  # just ls
./ls arg1   # just one argument
./ls -a  # just one flag
./ls -a arg1 # one flag, one argument
# order and format of flags
./ls -a -R  # multiple flags
./ls -R -a  # multiple flags, different order
./ls -aRl # multiple flags as one argument
./ls -Rla # multiple flags as one argument, different order
./ls -lR -a # multiple flags where at least two are in one argument
# order of flags and arguments
./ls -a arg1 -l   # argument between flags
./ls arg1 -a arg2 # flag between arguments

##### TEST THAT FILE ARGUMENTS ARE PROPERLY INTERPRETED
./ls normalfile rshell emptydir dir # test when some are files and some are directories
./ls dir/ # test directory names with / at the end
./ls .  # test .
./ls .. # test ..

##### TEST flags
./ls -l # show that -l works properly
./ls -a # show that -a works properly
./ls -R # show that -R works properly, and does not recurse infinitely through .  and ..
./ls -Ra # show that hidden directories are recursed into only when -a is used
./ls -alR # show that flags work together

##### COLORING
./ls # show that coloring works properly, as well as * and / at the end of executables and directories, respectively
./ls -l # show that coloring works properly with -l

##### TEST LARGE AMOUNTS OF FILES AND ALPHABETIZATION
mkdir largedir
for i in {a..z}; do touch largedir/$i$i$i; done;
for i in {A..Z}; do touch largedir/$i$i$i$i$i; done;
for i in {1..9}; do touch largedir/$i; done; # at this point there should be 61 files
./ls largedir # show that column output can handle many files, and show alphabetization. I use the built-in c++ sort.

##### ERRORS
./ls blorgobloop # file does not exist
./ls normalfile/ # file does not exist

#test ~
#test file names like ~//////Documents
#alphabetization and proper order
# . and .., but also check .files
# show that it does not recurse through . and ..
#test so many arguments that it spills onto the next line
#directories with / at the end and without / at the end
cd -
