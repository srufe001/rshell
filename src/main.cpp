/* TODO: rewrite with lex/yacc for easy extension in the future
 * is perror only for syscalls that can fail or should we use it for all error
 * messages?
 * is it even appropriate to use perror with these messages?
 * specifically, getlogin() does not mention serring errno
 *
 * Valgrind? I know I have leaks but they're followed by an execvp or exit
 * has problems with reachable: 187, 195 + 2
 * has problems with possibly: 69, 103, 190 +2
 *
 * TODO VERY VERY IMPORTANT - bash does it correctly, you do it incorrectly
 * in the command "fail || thing1 ; thing2" bash runs fail, thing2, but rshell
 * only runs fail
 *
 *
 * is it ok for fork failure to result in a simple return? (continue on with the
 * next command). I return 1 (error);
 *
 * wait can be triggered if child is affected by signal? What if this happens?
 *
 * using -std=c++11 vs -ansi? they seem to override and I may need c++11. I
 * should check this
 *
 * return value - what if WIFEXITED returns false?
 *
 * double check - is heap memory freed on execvp()? How about exit()?
 *
 * extra make targets? I have run for convenience but the assignment says 2
 *
 * extra files (such as .gitignore)
 *
 * docs: no command is success. unrecognized command is an error, and a failure
 * man doesn't let you look at the document, when rshell is run with ":make run"
 * from within vim
 * vim, however, does let you edit, although it warns you that output is "not to
 * a terminal". I guess rshell is not good enough for Bram
 * output of ls is in a column instead of in a row, if rshell is run with
 * ":make run" from within vim
 */

// comment for information about rshells internal stuff as it executes
// commands
#define NDEBUG

#include<iostream>
#include<string>
#include<cstdlib>
#include<cctype>
#include<vector>
#include<cassert>
#include<cstring>

#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>

using namespace std;

unsigned execute_command(vector<string>);

int main()
{
   // get username and hostname to print at the prompt
   char * usernameptr = getlogin();
   string username;
   if (usernameptr == 0)
   {
      perror("unable to get login information");
   } else {
      username.assign(usernameptr);
   }
   // I will print up to 16 characters of the hostname
   const int HOST_NAME_MAX = 255;      // POSIX guarantees it won't be longer than 255 bytes
   char hostname[HOST_NAME_MAX + 1];   // I make the array hold 256 bytes because I don't know
                                       // if the 255 bytes includes the null character
   if (-1 == gethostname(hostname, HOST_NAME_MAX + 1))
   {
      // if gethostname fails, print error message, and make hostname contain
      // nothing
      perror("unable to get hostname");
      hostname[0] = '\0';
   } else {
      // POSIX does not specify whether the last character of hostname is NULL
      // if the hostname is longer than the size of hostname, so I guarantee
      // that hostname has a terminating null character
      hostname[HOST_NAME_MAX] = '\0'; 
   }

   // this loop gets input from the user and executes it
   while (true)
   {
      // print prompt
      cout << username << "@" << hostname << "$ ";

      // get user input
      string userin;
      getline(cin, userin);
      assert(cout << "################ user inputted: " << userin << endl);

      // !!!divide up user input
      // slice off comment (everything after and including '#', if present)
      // TODO: you may have to move this down if you ever want to include "" or
      // ''
      userin = userin.substr(0, userin.find("#", 0)); 
      assert(cout << "######## after comment removal: " << userin << endl << endl);

      // current_command stores all white-space seperated strings found so far.
      // when a connector is found, current_command is used to execute a command and
      // is then erased in preparation for the next command
      // strings are added by adding each character to the last string in
      // current_command. When a whitespace is found, a new, empty string is
      // pushed to the back of current_command
      vector<string> current_command;
      // newWord is true when the last word has been completed and
      // the new one has not been started yet (such as after a whitespace or
      // connector like ;)
      bool newWord = true;
      // skipNext is true when the next command should be skipped (due to && or
      // ||) When this is true, the for loop will only check for connectors
      bool skipNext = false;
      // returnValue hold the exit code of the last run command
      int returnValue;
      // this loop parses the user's input and executes it
      for (unsigned i = 0; i < userin.size();)
      {
         // check for && connector
         if (userin.size() - i >= 2 && userin.at(i) == '&' && userin.at(i + 1) == '&')
         {
            // advance i
            i += 2;
            // execute command, update returnValue
            if (!skipNext)
               returnValue = execute_command(current_command);

            // erase current_command
            current_command.erase(current_command.begin(), current_command.end());
            newWord = true;
            // don't execute next command if the current command failed
            skipNext = (returnValue) ? true : false;
            assert(cout << "&&&&&&&&&&&&&&&&&&&&&&" << endl << endl);
         }
         // check for || connector
         else if (userin.size() - i >= 2 && userin.at(i) == '|' && userin.at(i + 1) == '|')
         {
            // advance i
            i += 2;
            // execute command, update returnValue
            if (!skipNext)
               returnValue = execute_command(current_command);

            // erase current_command
            current_command.erase(current_command.begin(), current_command.end());
            newWord = true;
            // skip the next command if the current command succeeded
            skipNext = (!returnValue) ? true : false;
            assert(cout << "||||||||||||||||||||||" << endl << endl);
         }
         // check for ; connector
         else if (userin.size() - i >= 1 && userin.at(i) == ';')
         {
            // advance i
            ++i;
            // execute command
            execute_command(current_command);
            // erase current_command
            current_command.erase(current_command.begin(), current_command.end());
            newWord = true;
            // the command after ; always executes
            skipNext = false;
            assert(cout << ";;;;;;;;;;;;;;;;;;;;;" << endl << endl);
            // continue regardless of return value
         } else if (skipNext)
         {
            ++i;
            // we've checked for connectors, we should skip the rest
            continue;  
         } else if (isspace(userin.at(i)))
         {
            // advance i
            ++i;
            newWord = true;
            continue;
         } else {
            // The character is not a connector, add it to current_command
            // add a new string to current_command if necessary
            if (newWord)
            {
               current_command.push_back(string());
               newWord = false;
            }
            current_command.back().push_back(userin.at(i));
            ++i;
         }
      }
      // done parsing input, execute whatever you have at this point
      execute_command(current_command);
      // the user's command has been completely executed by this point
   } /* end while */

   /* end main */
   return 0;
}

unsigned execute_command(vector<string>& command)
{
   if (command.size() == 0) return 0;  // no command returns true

   // chop off last string in command if it doesnt contain anything
   //if (command.back() == "")
   assert(cout << "==========COMMAND: ");
   for (unsigned i = 0; i < command.size(); ++i)
   {
      assert(cout << command.at(i) << " # ");
   }
   assert(cout << endl);

   // check for special builtin commands
   // check for "exit"
   if (command.at(0) == "exit")
   {
      cout << "exiting rshell..." << endl;
      exit(0);
   }

   // execute command as normal. begin by forking yourself
   int pid = fork();
   // fork failed
   if (pid == -1)
   {
      perror("forking failed");
      exit(1);
      //return 1; // see todo section at the top of this page
   }
   // fork succeeded, and you are the child
   if (pid == 0)
   {
      // convert command (a vector) into a null-terminated array of cstrings
      char ** argv = new char*[command.size() + 1];
      for (unsigned i = 0; i < command.size(); ++i)
      {
         argv[i] = new char[command.at(i).size() + 1];
         // copy the argument into the cstring
         for (unsigned j = 0; j < command.at(i).size() + 1; ++j)
         {
            argv[i][j] = command.at(i).c_str()[j];
         }
      }
      argv[command.size()] = NULL;
      execvp(argv[0], argv);
      // if we get to this point, execvp failed. print an error message
      char errorStr[80];
      strcpy(errorStr, "Failed to execute \"");
      if (strlen(argv[0]) > 50)
      {
         // I can screw with this string because no one's using it after this,
         // because I am doing exit(1)
         argv[0][47] = '.';
         argv[0][48] = '.';
         argv[0][49] = '.';
         argv[0][50] = '\0';
      }
      strcat(errorStr, argv[0]);
      strcat(errorStr, "\"");
      perror(errorStr);
      //IMPORTANT if you don't exit(1) (change it to return or whatever), you have to free memory allocated to argv
      exit(1);
      //return 1; // see todo section at the top of this page
   }
   // fork succeeded, and you are the parent
   int status;
   if (-1 == wait(&status))
   {
      perror("wait failed");
      exit(1);
   }
   // determine the return value of the child
   if (WIFEXITED(status))
   {
      assert(cout << "==========RETURNED: " << WEXITSTATUS(status) << endl << endl);
      return WEXITSTATUS(status);
   }
   assert(cout << "child did not return correctly" << endl << endl);
   return 1;   // if the child did not exit normally, assume that the return value is a fail
}
