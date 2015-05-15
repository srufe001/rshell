/* TODO: rewrite with lex/yacc for easy extension in the future
 * is perror only for syscalls that can fail or should we use it for all error
 * messages?
 * is it even appropriate to use perror with these messages?
 * specifically, getlogin() does not mention serring errno
 *
 * wait can be triggered if child is affected by signal? What if this happens?
 *
 * return value - what if WIFEXITED returns false?
 *
 * docs: no command is success. unrecognized command is an error, and a failure
 * man doesn't let you look at the document, when rshell is run with ":make run"
 * from within vim
 * vim, however, does let you edit, although it warns you that output is "not to
 * a terminal". I guess rshell is not good enough for Bram
 * output of ls is in a column instead of in a row, if rshell is run with
 * ":make run" from within vim
 */

/*  How bash handles redirection:
 *  pipes redirect stdout and stderr
 *  by default, < and > only redirect stderr. see &>, 2>, etc.
 *  redirection involving < and > has priority over pipes.
 *  in redirection involving < and >, the rightmost has precedence.
 *  However, other files that dont get written to get created anyways
 *  This means that pipes are created before, and that < and > to the left are
 *  created, then the < and > on the right
 *
 *  for custom redirection (&>, 2>, etc.) whenever we find a > inside a word, we check the
 *  previous word to see if its a valid fd (a number, or &). I'm going to assume
 *  that &>, &1, etc., do not need to be implemented
 *
 *  TODO is newWord really necessary?
 *  consider using `list` for `current_command`
 *  exit vs _exit
 *  add closing to the &&
 *  remove couts, uncomment asserts as well maybe
 *  improve error message for "| command"
 *  still getting WIFEXITED FAILED on single command after "| |"
 */

// comment the next line for information about rshells internal stuff as it executes commands
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
bool isAFd(const string);

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
      cout << "\x1b[32m" << username << "@" << hostname << "$ " << "\x1b[0m";

      // get user input
      string userin;
      getline(cin, userin);
      //assert(cerr << "################ user inputted: " << userin << endl);

      // ==============================
      //       Process user input
      // ==============================
      // slice off comment (everything after and including '#', if present)
      // TODO: you may have to move this down if you ever want to include "" or
      // ''
      userin = userin.substr(0, userin.find("#", 0)); 
      //assert(cerr << "######## after comment removal: " << userin << endl << endl);

      // current_command stores all white-space seperated strings found so far.
      // when a connector is found, current_command is used to execute a command and
      // is then erased in preparation for the next command
      // strings are added by adding each character to the last string in
      // current_command. When a whitespace, or other special character (;, &&,
      // ||, |, <, >, >>, >>>) is found, a new, empty string is pushed to the
      // back of current_command
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
            i += 2;
            // execute command, update returnValue
            if (!skipNext)
               returnValue = execute_command(current_command);

            // erase current_command
            current_command.erase(current_command.begin(), current_command.end());
            newWord = true;
            // don't execute next command if the current command failed
            skipNext = (returnValue) ? true : false;
            //assert(cerr << "&&&&&&&&&&&&&&&&&&&&&&" << endl << endl);
         }
         // check for || connector
         else if (userin.size() - i >= 2 && userin.at(i) == '|' && userin.at(i + 1) == '|')
         {
            i += 2;
            // execute command, update returnValue
            if (!skipNext)
               returnValue = execute_command(current_command);

            // erase current_command
            current_command.erase(current_command.begin(), current_command.end());
            newWord = true;
            // skip the next command if the current command succeeded
            skipNext = (!returnValue) ? true : false;
            //assert(cerr << "||||||||||||||||||||||" << endl << endl);
         }
         // check for ; connector
         else if (/*TODO remove userin.size() - i >= 1 && */userin.at(i) == ';')
         {
            ++i;
            // execute command
            returnValue = execute_command(current_command);
            // erase current_command
            current_command.erase(current_command.begin(), current_command.end());
            newWord = true;
            // the command after ; always executes
            skipNext = false;
            //assert(cerr << ";;;;;;;;;;;;;;;;;;;;;" << endl << endl);
            // continue regardless of return value
         } else if (skipNext)
         {
            ++i;
            // we've checked for connectors, we don't need to check for anything
            // else, because the current command should not be executed because
            // of an earlier connector being unsatisfied
            continue;  
         }
         else if (isspace(userin.at(i)))
         {
            ++i;
            newWord = true;
         }
         else if (userin.at(i) == '|')
         {
            ++i;
            current_command.push_back(string("|"));
            newWord = true;
         }
         else if (userin.size() - i >= 2 && userin.at(i) == '>' && userin.at(i + 1) == '>')
         {
            i += 2;
            // check if there might be a file descriptor preceding the >>
            if (!newWord)
            {
               if (isAFd(current_command.back()))
               {
                  string s = current_command.back();
                  current_command.back() = string(">");
                  current_command.push_back(s);
               }
            }
            current_command.back() = string(">");
            current_command.push_back(string("1"));
            newWord = true;
         }
         // > is stores in `current_command` as a string containing ">" followed
         // by a string containing the file descriptor to be redirected
         else if (userin.at(i) == '>')
         {
            ++i;
            // check if there might be a file descriptor preceding the >
            if (!newWord)
            {
               if (isAFd(current_command.back()))
               {
                  string s = current_command.back();
                  current_command.back() = string(">");
                  current_command.push_back(s);
               }
            }
            current_command.back() = string(">");
            current_command.push_back(string("1"));
            newWord = true;
         }
         else if (userin.at(i) == '<')
         {
            ++i;
            current_command.push_back(string("<"));
            newWord = true;
         }
         else {
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

// runs the command specified by `command`
// accepts a container of strings that contains the command to run, its
// arguments, and any redirection done with >, <, or >>
// also accept pipeRead, and pipeWrite, which represent pipes to read from and
// write to, respectively. If the command shouldn't write or read from a pipe,
// give the appropriate argument the value -1
unsigned execute_command(vector<string> command)
{
   if (command.size() == 0) return 0;  // no command returns true

   // chop off last string in command if it doesnt contain anything
   //if (command.back() == "")

   if (command.back() == "|")
   {
      cerr << "rshell: You must have a program to run after each pipe\n";
      return 1;
   }

   //assert(cerr << "==========LINE: ");
#ifndef NDEBUG
   for (unsigned i = 0; i < command.size(); ++i)
   {
      //assert(cerr << command.at(i) << " # ");
   }
   //assert(cerr << endl);
#endif

   // begin executing the commands

   // childpids includes the pid of every child
   vector<int> childpids;
   // `pipes` contains two pairs of file descriptors to pipes. One contains
   // the pipe that the next command should read from, the other contains the
   // pipe that the next command should write to. Which pipe is which is
   // determined by `readpipe`. The read pipe is `pipes[readpipe]`, the write
   // pipe is `pipes[writepipe]. readpipe changes from true (1) to false (0)
   // every time a command is executed and a pipe connects it to another
   // command.
   int pipes[2][2] = {-1, -1, -1, -1};
   //assert(pipes[0][0] == -1 && pipes[0][1] == -1 && pipes[1][0] == -1 && pipes[1][1] == -1);
   bool readpipe = true;   // the initial value of readpipe does not matter
#define writepipe !readpipe
#define readend pipes[readpipe][0]
#define writeend pipes[!readpipe][1]
   // commandstart and commandend store the indices, inclusive, of the command
   // to execute
   unsigned commandstart = 0;
   unsigned commandend;
   for (commandend = 0; commandend < command.size(); ++commandend)
   {
      if (command.at(commandend) == "|" || commandend == command.size() - 1)
      {
         // don't include the pipe symbol
         if (command.at(commandend) == "|")
         {
            --commandend; 
         }
         // check for the following case: "cmd1 | | cmd2", or when `command`
         // starts with "|"
         if (command.at(commandstart) == "|")
         {
            cerr << "rshell: you cannot pipe directly into another pipe\n";
            break;
         }

         // swap read and write pipes
         readpipe = !readpipe;
         // create a new pipe to write to if appropriate
         if (commandend != command.size() - 1)
         {
            if (-1 == pipe(pipes[writepipe]))
            {
               perror("pipe");
               return 1;
            }
         }

         //assert(cerr << "pipevars contain: " << pipes[readpipe][0] << " "<< pipes[readpipe][1] << " "<< pipes[writepipe][0] << " "<< pipes[writepipe][1] << " " << endl);

         // check for special builtin commands
         // check for "exit"
         if (command.at(commandstart) == "exit")
         {
            exit(0);
         }

         // execute command as normal. begin by forking yourself
         int pid = fork();
         // fork failed
         if (pid == -1)
         {
            perror("forking failed");
            return 1;   // return that the command failed
         }
         // fork succeeded, and you are the child
         if (pid == 0)
         {
            // these variables represent the fds that should be `dup`ed into
            // stdin, stdout, stderr
            // initially they are the current fds of the pipes. It is ok if the
            // pipe has not been created and has value -1
            int in = pipes[readpipe][0];
            int out = pipes[writepipe][1];
            int err = out;
            // dup fds into stdin, stdout, stderr, TODO close old fds.
            if (in != -1)
            {
               if (-1 == dup2(in, 0))
               {
                  perror("dup2");
                  return 1;
               }
               if (-1 == close(in))
                  perror("close");
            }
            if (out != -1)
            {
               if (-1 == dup2(out, 1))
               {
                  perror("dup2");
                  return 1;
               }
               if (err != out)
               {
                  if (-1 == close(out))
                     perror("close");
               }
            }
            if (err != -1)
            {
               if (-1 == dup2(err, 2))
               {
                  perror("dup2");
                  return 1;
               }
               if (-1 == close(err))
                  perror("close");
            }

            //assert(cerr << "pipe fds: " << in << " " << out << " " << err << endl);
            //assert(cerr << commandstart << " " << commandend << endl);

            // convert command (a vector) into a null-terminated array of cstrings,
            // resolving input/output redirection from <, >, >> as you go along (TODO)
/*#define*/ unsigned argc = commandend - commandstart + 2;
            //assert(cerr << "argc: " << argc << endl);
            char ** argv = new char*[argc];
            for (unsigned i = commandstart; i <= commandend; ++i)
            {
               argv[i - commandstart] = new char[command.at(i).size() + 1];
               // copy the argument into the cstring TODO replace with strcpy?
               strcpy(argv[i - commandstart], command.at(i).c_str());
            }
            argv[argc - 1] = NULL;

            // TODO remove
            cout << "argv: ";
            for (unsigned i = 0; i < argc - 1; ++i)
            {
               cout << argv[i] << " # ";
            }
            cout << endl;


            // execute the command
            execvp(argv[0], argv);

            // if we get to this point, execvp failed. print an error message
            char errorStr[80];
            strcpy(errorStr, "Failed to execute \"");
            // truncate argv[0] to 50 characters
            // I don't know the max so I can't risk overflowing errorStr
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
            exit(1);
         }
         // fork succeeded, and you are the parent
         childpids.push_back(pid);  // record the child's pid to wait on it later

         // close unnecessary pipes (pipes[writepipe][1], pipes[readpipe][0]),
         // and set them to -1
         if (pipes[writepipe][1] != -1)
         {
            if (-1 == close(pipes[writepipe][1]))
               perror("closing writepipe 1");
            pipes[writepipe][1] = -1;
         }
         if (pipes[readpipe][0] != -1)
         {
            if (-1 == close(pipes[readpipe][0]))
               perror("closing readpipe 0");
         }

         // advance commandend and commandstart
         ++commandend;
         commandstart = commandend + 1;
      }
   }

   int status;
   bool lastchildreturned = false;
   for (unsigned i = 0; i < childpids.size(); ++i)
   {
      int stat;
      int childpid;
      if (-1 == (childpid = wait(&stat)))
      {
         perror("wait failed");
         exit(1);
      } else if (childpid == childpids.back())
      {
         // update status iff the waited-on child was the final command
         status = stat;
         lastchildreturned = true;
      }
      assert(cerr << "waited on child " << childpid << ", child was " << ((childpid == childpids.back()) ? "last" : "not last") << endl);
   }
   if (!lastchildreturned)
      return 1;
   // determine the return value of the last child
   if (WIFEXITED(status))
   {
      //assert(cerr << "==========RETURNED: " << WEXITSTATUS(status) << endl << endl);
      return WEXITSTATUS(status);
   }
   cerr << "WIFEXITED failed\n";
   return 1;   // if the child did not exit normally, assume that the return value is a fail
}

bool isAFd(const string s)
{
   for (unsigned i = 0; i < s.size(); ++i)
   {
      if (s.at(i) < '0' || s.at(i) > '9')
         return false;
   }
   return true;
}
