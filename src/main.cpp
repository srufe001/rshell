/* TODO: rewrite with lex/yacc for easy extension in the future
 * is perror only for syscalls that can fail or should we use it for all error
 * messages?
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
 * docs: no command is success. unrecognized command is an error, and a failure
 * man doesn't let you look at the document
 * vim, however, does let you edit, although it warns you that output is "not to
 * a terminal". I guess rshell is not good enough for Bram
 * output of ls is in a column instead of in a row
 */

// comment for information about rshells internal stuff as it executes
// commands
#define NDEBUG

#include<iostream>
#include<string>
#include<cstdlib>
#include<cctype>
#include<vector>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<cassert>

using namespace std;

unsigned execute_command(vector<string>);

int main()
{
   while (true)
   {
      // print prompt
      cout << "$ ";

      // get user input
      string userin;
      getline(cin, userin);
      //assert(cout << "user inputted: " << userin << endl);

      // !!!divide up user input
      // slice off comment (everything after and including '#', if present)
      // TODO: you may have to move this down if you ever want to include "" or
      // ''
      userin = userin.substr(0, userin.find("#", 0)); 
      //assert(cout << "after comment removal: " << userin << endl);

      // current_command stores all white-space seperated strings found so far.
      // when a connector is found, current_command is used to execute a command and
      // is then emptied
      // strings are added by adding each character to the last string in
      // current_command. When a whitespace is found, a new, empty string is
      // pushed to the back of current_command
      vector<string> current_command;
      // newWord is true when the last word has been completed and
      // the new one has not been started
      bool newWord = true;
      for (unsigned i = 0; i < userin.size();)
      {
         if (isspace(userin.at(i)))
         {
            // advance i
            ++i;
            newWord = true;
            continue;
         }
         // check for && connector
         if (userin.size() - i >= 2 && userin.at(i) == '&' && userin.at(i + 1) == '&')
         {
            // advance i
            i += 2;
            // execute command
            int returnValue = execute_command(current_command);

            // erase current_command
            current_command.erase(current_command.begin(), current_command.end());
            // don't continue if the command failed
            if (returnValue) break;
            newWord = true;
            assert(cout << "&&&&&&&&&&&&&&&&&&&&&&" << endl << endl);
         }
         // check for || connector
         else if (userin.size() - i >= 2 && userin.at(i) == '|' && userin.at(i + 1) == '|')
         {
            // advance i
            i += 2;
            // execute command
            int returnValue = execute_command(current_command);

            // erase current_command
            current_command.erase(current_command.begin(), current_command.end());
            newWord = true;
            // don't continue if the command succeeded
            if (!returnValue) break;
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
            assert(cout << ";;;;;;;;;;;;;;;;;;;;;" << endl << endl);
            // continue regardless of return value
         } else {
            // add a new string to current_command if necessary
            if (newWord)
            {
               current_command.push_back(string());
               newWord = false;
            }
            // The character is not a token, so add it to current_command
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

unsigned execute_command(vector<string> command)
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
      perror("fork");
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
      perror("execvp");
      exit(1);
      //return 1; // see todo section at the top of this page
   }
   // fork succeeded, and you are the parent
   int status;
   if (-1 == wait(&status))
   {
      perror("wait");
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
