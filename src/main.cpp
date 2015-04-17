/* TODO: rewrite with lex/yacc for easy extension in the future
 * is perror only for syscalls that can fail or should we use it for all error
 * messages?
 *
 *
 * docs: no command is success. unrecognized command is an error, and a failure
 */

#include<iostream>
#include<string>
#include<cstdlib>
#include<cctype>
#include<vector>

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
      cout << "user inputted: " << userin << endl;

      // !!!divide up user input
      // slice off comment (everything after and including '#', if present)
      userin = userin.substr(0, userin.find("#", 0)); 
      cout << "after comment removal: " << userin << endl;

      // current_command stores all white-space seperated strings found so far.
      // when a connector is found, current_command is used to execute a command and
      // is then emptied
      // strings are added by adding each character to the last string in
      // current_command. When a whitespace is found, a new, empty string is
      // pushed to the back of current_command
      vector<string> current_command;
      current_command.push_back(string());
      for (unsigned i = 0; i < userin.size();)
      {
         if (isspace(userin.at(i)))
         {
            // advance i
            ++i;
            //add a new string to current_command if necessary
            if (current_command.back() != "")
            {
               current_command.push_back(string());
            }
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
            current_command.push_back(string());
            cout << "CONNECTOR was &&" << endl;
            // don't continue if last command failed
            if (returnValue) break;
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
            current_command.push_back(string());
            cout << "CONNECTOR was ||" << endl;
            // don't continue if last command failed
            if (!returnValue) break;
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
            current_command.push_back(string());
            cout << "CONNECTOR was ;" << endl;
            // continue regardless of return value
         } else {
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
   cout << "printing individual submitted strings:" << endl;
   for (unsigned i = 0; i < command.size(); ++i)
   {
      cout << command.at(i) << endl;
   }
   return 0;

   /*
   if (command.size() == 0) return 0;  // no command is a true command

   // check for special builtin commands
   if (command.at(i) == "exit")
   {
      // cout something maybe?
      exit(0);
   }

   // execute command as normal
   int forkReturn = fork();
   // fork failed
   if (forkReturn == -1)
   {
      perror();
      // cout something
      // cout something if perror is not enough?
      return 1;
   }
   // fork succeeded, and you are the child
   if (forkReturn == 0)
   {
      execvp();
      perror();
      // cout something if perror is not enough?
      return 1;
   }
   // fork succeeded, and you are the parent
   int returnValue;
   wait();
   return returnValue;
   */
}
