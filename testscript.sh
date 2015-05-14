#!/bin/bash
set -x
PS4="srufe001 $ "
echo $PS1
ls
pwd

echo "testing out rshell"
#teststr="ls\npwd\nexit\n"
bin/rshell <<< $'ls\npwd\nexit\n'
