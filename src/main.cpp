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
 *  FIXED (Don't ever "return 1" in the child! use exit(1)) "ls >" or ls ">>", after this "exit" does not work properly
 *  take a look at writing to a file with multiple different processes. "cat <<<
 *  "hi" > bla.txt | cat <<< "there" > bla.txt"
 *
 */

/*  signals and pwd:
 *  check that the ~ substitution for the prompt works when it should be ~.*, ~,
 *  or just the pwd (this is when you're not in the home directory)
 *  can't use "cd -" after "cd"?
 */
// comment the next line for information about rshells internal stuff as it executes commands
#define NDEBUG
// uncomment this line if you dont want color codes (such as making a script
// file
#define COLOR_PROMPT
// uncomment this line if you're making a script file
//#define ECHO_COMMAND

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
#include<fcntl.h>
#include<signal.h>
#include<errno.h>

using namespace std;

bool killChildren; // SIGINT sets this to true
bool suspendChildren; // SIGTSTP sets this to true
// `childpids` includes the pid of every child, in order, such that
// `childpids.back()` contains the pid of the last child
vector<int> childpids;
// this vector holds pids of the last suspended job
vector<int> suspendedchildpids;

unsigned execute_command(vector<string>);
bool isAFd(const string&);
void changedir(const string&);
void SIGINThandler(int);
void SIGTSTPhandler(int);

int main()
{
   // register signal handlers
   struct sigaction s_sigint;
   s_sigint.sa_handler = SIGINThandler;
   if (-1 == sigaction(SIGINT, &s_sigint, NULL))
   {
      perror("sigaction");
   }
   struct sigaction s_sigtstp;
   s_sigtstp.sa_handler = SIGTSTPhandler;
   if (-1 == sigaction(SIGTSTP, &s_sigtstp, NULL))
   {
      perror("sigaction");
   }
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
      killChildren = false;
      suspendChildren = false;
      /* Output Prompt */
      // get pwd
      char * pwdptr = getenv("PWD");
      string pwd;
      if (pwdptr == NULL)
         perror("could not get PWD");
      else
         pwd = pwdptr;
      // if the pwd contains $HOME, replace it with "~"
      char * home = getenv("HOME");
      if (home == NULL)
      {
         perror("could not get home directory");
      } else if (pwd.size() >= strlen(home))
      {
         if (pwd.substr(0, strlen(home)) == home)
         {
            pwd = pwd.substr(strlen(home) - 1);
            pwd.at(0) = '~';
         }
      }
      // print prompt
#ifdef COLOR_PROMPT
      cout << "\x1b[35m" << username << "@" << hostname << ":" << pwd << "$ " << "\x1b[0m";
#else
      cout  << username << "@" << hostname << ":" << pwd << "$ ";
#endif

      /* Get User Input */
      string userin;
      cin.clear();
      getline(cin, userin);
      //assert(cerr << "################ user inputted: " << userin << endl);
#ifdef ECHO_COMMAND 
      cout << userin << "\n";
#endif

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
      // ||, |, <, >, >>, <<< is found, a new, empty string is pushed to the
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
         // >> is stored in `current_command` as a string containing ">>" followed
         // by a string containing the file descriptor to be redirected
         else if (userin.size() - i >= 2 && userin.at(i) == '>' && userin.at(i + 1) == '>')
         {
            i += 2;
            // check if there might be a file descriptor preceding the >>
            string s;
            if (!newWord)
            {
               if (isAFd(current_command.back()))
               {
                  s = current_command.back();
                  current_command.erase(current_command.end() - 1);
               }
            }
            current_command.push_back(string(">>"));
            if (s.size() > 0)
            {
               current_command.push_back(s);
            } else {
               current_command.push_back(string("1"));
            }
            newWord = true;
         }
         // > is stored in `current_command` as a string containing ">" followed
         // by a string containing the file descriptor to be redirected
         else if (userin.at(i) == '>')
         {
            ++i;
            // check if there might be a file descriptor preceding the >
            string s;
            if (!newWord)
            {
               if (isAFd(current_command.back()))
               {
                  s = current_command.back();
                  current_command.erase(current_command.end() - 1);
               }
            }
            current_command.push_back(string(">"));
            if (s.size() > 0)
            {
               current_command.push_back(s);
            } else {
               current_command.push_back(string("1"));
            }
            newWord = true;
         }
         else if (userin.size() - i >= 3 && userin.at(i) == '<' && userin.at(i + 1) == '<' && userin.at(i + 1) == '<')
         {
            i += 3; 
            current_command.push_back(string("<<<"));
            newWord = true;
         }
         else if (userin.at(i) == '<')
         {
            ++i;
            current_command.push_back(string("<"));
            newWord = true;
         }
         else {
            // The character is not a connector or redirection
            if (newWord)
            {
               current_command.push_back(string());
               newWord = false;
            }
            // allow backslash escaping of "
            if (userin.at(i) == '\\' && i < userin.size() - 1 && userin.at(i + 1) == '\"')
            {
               current_command.back().push_back(userin.at(i + 1)); 
               i += 2;
            }
            else if (userin.at(i) == '\"')
            {
               // if the character is a ", process everything literally until next
               // unescaped "
               for (++i; i < userin.size() && userin.at(i) != '\"'; ++i)
               {
                  // allow backslash escaping of "
                  if (userin.at(i) == '\\' && i < userin.size() - 1 && userin.at(i + 1) == '\"')
                  {
                     ++i;
                     current_command.back().push_back(userin.at(i)); 
                  } else {
                     current_command.back().push_back(userin.at(i)); 
                  }
               }
               ++i;
            } else {
               current_command.back().push_back(userin.at(i));
               ++i;
            }
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
   if (killChildren || suspendChildren) return 1; // dont make new children if you want to kill them (because of sigint)
   if (command.size() == 0) return 0;  // no command returns true

   if (command.back() == "|")
   {
      cerr << "rshell: You must have a program to run after each pipe\n";
      return 1;
   }

   /* Begin executing the commands */

   // `pipes` contains two pairs of file descriptors to pipes. One contains
   // the pipe that the next command should read from, the other contains the
   // pipe that the next command should write to. Which pipe is which is
   // determined by `readpipe`. The read pipe's fds are in `pipes[readpipe]`, the write
   // pipe's fds are in `pipes[writepipe]. `readpipe` changes from true (1) to false (0)
   // every time a command is executed and a pipe connects it to another
   // command.
   int pipes[2][2] = {-1, -1, -1, -1};
   bool readpipe = true;   // the initial value of readpipe does not matter
   #define writepipe !readpipe
   // commandstart and commandend store the indices, inclusive, of the command
   // to execute
   unsigned commandstart = 0;
   unsigned commandend;
   bool stopexecuting = false;
   for (commandend = 0; commandend < command.size() && !killChildren && !stopexecuting; ++commandend)
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
            if (commandstart > 0)
            {
               cerr << "rshell: you cannot pipe directly into another pipe\n";
            } else {
               cerr << "rshell: you cannot start a command with a pipe\n";
            }
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

         // setting this to true prevents forking and execution of any command
         bool builtin = false;
         /* Check for special builtin commands */
         if (command.at(commandstart) == "cd" )
         {
            builtin = true;
            // check usage
            if (commandend - commandstart > 1)
            {
               cout << "cd takes at most 1 argument\n";
            } else
            {
               // determine the path to change directory to
               if (commandend == commandstart) // just "cd"
               {
                  char * homeptr = getenv("HOME");
                  if (NULL == homeptr)
                  {
                     perror("getenv");
                  } else 
                  {
                     string newdir(homeptr);
                     changedir(newdir);
                  }
               } else
               {
                  changedir(command.at(commandstart + 1));
               }
            }
         } else if (command.at(commandstart) == "fg")
         {
            builtin = true;
            stopexecuting = true;
            // make sure that there aren't other commands that need to be waited
            // on
            if (commandstart != 0)
            {
               cout << "you may not use redirection with fg\n";
            } else
            {
               for (unsigned j = 0; j < suspendedchildpids.size(); ++j)
               {
                  kill(suspendedchildpids.at(j), SIGCONT);
                  childpids.push_back(suspendedchildpids.at(j));
               }
               suspendedchildpids.clear();
            }
         } else if (command.at(commandstart) == "bg")
         {
            builtin = true;
            stopexecuting = true;
            // make sure that there aren't other commands that need to be waited
            // on
            if (commandstart != 0)
            {
               cout << "you may not use redirection with bg\n";
            } else
            {
               for (unsigned j = 0; j < suspendedchildpids.size(); ++j)
               {
                  kill(suspendedchildpids.at(j), SIGCONT);
               }
               suspendedchildpids.clear();
            }
         } else if (command.at(commandstart) == "exit")
         {
            exit(0);
         }
         
         if (!builtin)
         {
            // execute command as normal. begin by forking yourself
            int pid = fork();
            // fork failed
            if (pid == -1)
            {
               perror("forking failed");
               return 1;
            }
            /* Fork succeeded, and you are the child */
            if (pid == 0)
            {
               /* Prepare child's fds for piping */
               // dup fds for pipes into stdin, stdout, stderr
               if (pipes[readpipe][0] != -1)
               {
                  if (-1 == dup2(pipes[readpipe][0], 0))
                  {
                     perror("dup2");
                     exit(1);
                  }
               }
               if (pipes[writepipe][1] != -1)
               {
                  if (-1 == dup2(pipes[writepipe][1], 1))
                  {
                     perror("dup2");
                     exit(1);
                  }
                  if (-1 == dup2(pipes[writepipe][1], 2))
                  {
                     perror("dup2");
                     exit(1);
                  }
               }
               //close any pipe-related fds remaining (besides 0, 1, 2, of course)
               //pipes[readpipe][1] does not need to be closed because it was
               //already closed, or never existed at all
               if (pipes[readpipe][0] != -1)
               {
                  if (-1 == close(pipes[readpipe][0]))
                     perror("close 1");
                  pipes[readpipe][0] = -1;
               }
               if (pipes[writepipe][0] != -1)
               {
                  if (-1 == close(pipes[writepipe][0]))
                     perror("close 2");
                  pipes[writepipe][0] = -1;
               }
               if (pipes[writepipe][1] != -1)
               {
                  if (-1 == close(pipes[writepipe][1]))
                     perror("close 3");
                  pipes[writepipe][1] = -1;
               }

               //assert(cerr << "pipe fds: " << in << " " << out << " " << err << endl);
               //assert(cerr << commandstart << " " << commandend << endl);

               /* Build `argv` from `command` and handle redirection involving <, >, >> */
               unsigned argc = 0;
               char ** argv = new char*[commandend - commandstart + 2];
               for (unsigned i = commandstart; i <= commandend; ++i)
               {
                  // check if next command is a redirection, and handle it if it is
                  if (command.at(i) == "<")
                  {
                     if (i <= commandend - 1)
                     {
                        if (-1 == close(0))
                        {
                           perror("close");
                           exit(1);
                        }
                        // open will use fd 0, because I just closed it
                        if (-1 == open(command.at(i + 1).c_str(), O_RDONLY)) 
                        {
                           perror("open");
                           exit(1);
                        }
                        ++i;
                        continue;
                     } else {
                        cerr << "you must give `<' a file to read from" << endl;
                        exit(1);
                     }
                  }
                  if (command.at(i) == "<<<")
                  {
                     if (i <= commandend - 1)
                     {
                        // create a pipe to hold the string
                        int stringholder[2];
                        if (-1 == pipe(stringholder))
                        {
                           perror("pipe");
                           exit(1);
                        }
                        // add a newline to the user's input
                        char *userstr = new char[command.at(i + 1).size() + 1];
                        strcpy(userstr, command.at(i + 1).c_str());
                        userstr[command.at(i + 1).size()] = '\n';
                        if (-1 == write(stringholder[1], userstr, command.at(i + 1).size() + 1))
                        {
                           perror("write");
                           exit(1); 
                        }
                        delete[] userstr;
                        if (-1 == close(stringholder[1]))
                        {
                           perror("close");
                           exit(1);
                        }
                        if (-1 == dup2(stringholder[0], 0))
                        {
                           perror("dup2");
                           exit(1);
                        }
                        ++i;
                        continue;
                     } else {
                        cerr << "you must give `<<<' a string to use as input" << endl;
                        exit(1);
                     }
                  }
                  if (command.at(i) == ">")
                  {
                     if (i <= commandend - 2)
                     {
                        int targetfd = atoi(command.at(i + 1).c_str());
                        int initialfd = open(command.at(i + 2).c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); 
                        if (-1 == initialfd)
                        {
                           perror("open");
                           exit(1);
                        }
                        if (-1 == dup2(initialfd, targetfd))
                        {
                           perror("dup2");
                           exit(1);
                        }
                        if (initialfd != targetfd)
                        {
                           if (-1 == close(initialfd))
                           {
                              perror("close");
                           }
                        }
                        i += 2;
                        continue;
                     } else {
                        cerr << "rshell: You must give `>' a file to redirect into" << endl;
                        exit(1);
                     }
                  }
                  if (command.at(i) == ">>")
                  {
                     if (i <= commandend - 2)
                     {
                        int targetfd = atoi(command.at(i + 1).c_str());
                        int initialfd = open(command.at(i + 2).c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR); 
                        if (-1 == initialfd)
                        {
                           perror("open");
                           exit(1);
                        }
                        if (-1 == dup2(initialfd, targetfd))
                        {
                           perror("dup2");
                           exit(1);
                        }
                        if (initialfd != targetfd)
                        {
                           if (-1 == close(initialfd))
                           {
                              perror("close");
                           }
                        }
                        i += 2;
                        continue;
                     } else {
                        cerr << "rshell: You must give `>>' a file to redirect into" << endl;
                        exit(1);
                     }
                  }
                  argv[argc] = new char[command.at(i).size() + 1];
                  strcpy(argv[argc], command.at(i).c_str());
                  ++argc;
               }
               argv[argc] = NULL;

               /* Execute the command */
               execvp(argv[0], argv);

               /* `execvp` failed, print error message and kill child */
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
            /* fork succeeded, and you are the parent */

            childpids.push_back(pid);  // record the child's pid to wait on it later
         }

         // close unnecessary pipes (pipes[writepipe][1], pipes[readpipe][0]),
         // and set them to -1. These pipes are connected to the child that just
         // executed, so we dont need to keep track of them anymore
         // pipes[writepipe][0] stays open because the next command (should one
         // exist) will need to read from it
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
            pipes[readpipe][0] = -1;
         }

         // advance commandend and commandstart
         ++commandend;
         commandstart = commandend + 1;
      }
   }
   // kill children if necessary. this is for the case where SIGINT is caught
   // during execute_command, so some children are created after the SIGINT
   // handler raises SIGINT on them. we need to kill these children
   if (killChildren)
   {
      for (unsigned i = 0; i < childpids.size(); ++i)
      {
         if (-1 == kill(childpids.at(i), SIGINT))
         {
            perror("kill");
         }
      }
   }

   /* Waiting on children, collecting exit status */
   int status;
   bool lastchildreturned = false;
   //cout << "beginning to wait on children" << endl;
   for (unsigned i = 0; i < childpids.size();)
   {
      if (suspendChildren)
      {
         // raise SIGTSTP and move to suspendedchildpids all unwaited children
         suspendedchildpids.clear();
         for (unsigned j = i; j < childpids.size(); ++j)
         {
            cout << "suspending " << childpids.at(j) << endl;
            kill(childpids.at(j), SIGTSTP);
            suspendedchildpids.push_back(childpids.at(j));
         }
         break;
      }
      int stat;
      int childpid;
      if (-1 == (childpid = wait(&stat)))
      {
         //cout << "wait failed\n";
         if (errno != EINTR)
         {
            perror("wait failed");
            exit(1);
         }
         //cout << "wait got interrupted\n";
         continue;
      }
      //cout << "wait did not fail" << endl;
      if (childpid == childpids.back())
      {
         // update status iff the waited-on child was the final command
         status = stat;
         lastchildreturned = true;
      }
      ++i;
      assert(cerr << "waited on child " << childpid << ", child was " << ((childpid == childpids.back()) ? "last" : "not last") << endl);
   }
   childpids.clear();
   killChildren = false;
   suspendChildren = false;
   //cout << childpids.size() << suspendedchildpids.size() << endl;
   if (!lastchildreturned)
      return 1;
   // determine the return value of the last child
   if (WIFEXITED(status))
   {
      //assert(cerr << "==========RETURNED: " << WEXITSTATUS(status) << endl << endl);
      return WEXITSTATUS(status);
   }
   //cerr << "WIFEXITED failed\n";
   return 1;   // if the child did not exit normally, assume that the return value is a fail
}

bool isAFd(const string& s)
{
   for (unsigned i = 0; i < s.size(); ++i)
   {
      if (s.at(i) < '0' || s.at(i) > '9')
         return false;
   }
   return true;
}

void changedir(const string& s)
{
   // check for "-"
   string newdir;
   if (s == "-")
   {
      char * oldpwdptr = getenv("OLDPWD");
      if (NULL == oldpwdptr)
      {
         perror("getenv");
      } else
      {
         newdir = oldpwdptr;
      }
   } else
   {
      newdir = s;
   }
   // change directory, update PWD and OLDPWD
   if (-1 == chdir(newdir.c_str()))
   {
      perror("chdir");
      return;
   }
   char * pwdptr = getenv("PWD");
   if (NULL == pwdptr)
   {
      perror("getenv PWD");
      return;
   }
   if (-1 == setenv("OLDPWD", pwdptr, 1))
   {
      perror("setenv OLDPWD");
   }
   char * newdirfullpath = get_current_dir_name();
   if (-1 == setenv("PWD", newdirfullpath, 1))
   {
      perror("setenv PWD");
   }
   free(newdirfullpath);
}

unsigned counter = 0;
void SIGINThandler(int s)
{
   ++counter;
   if (counter == 20) exit(0);
   cout << "you pressed ctrl-c";
   cout << "\n";
   killChildren = true;
   for (unsigned i = 0; i < childpids.size(); ++i)
   {
      if (-1 == kill(childpids.at(i), SIGINT))
      {
         perror("kill");
      }
   }
   return;
}

void SIGTSTPhandler(int s)
{
   ++counter;
   if (counter == 20) exit(0);
   cout << "you pressed ctrl-z";
   cout << "\n";
   suspendChildren = true;
   return;
}
