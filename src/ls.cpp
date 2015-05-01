//TODO: look up how ls prints in columns
//Do I have to delete stuff like the stat object

#include<iostream>
#include<unistd.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<vector>
#include<algorithm>
#include<string>

using namespace std;

#define PRINTARR(arr, num_elements) { for (int i = 0; i < num_elements; ++i) \
                                         cout << arr[i] << " "; \
                                      cout << endl; } \

bool show_hidden = false;
bool recursive = false;
bool more_info = false;

/* Potential helper functions */// - remember to check if c++ already has these
// string withColor(const string& file, struct stat s)
// padding function
// columns function!!!
//    see cout << setw(x) << left/right
//    You will still need to find the column widths, but this'll be easy for -l
//    For not -l, how can I tell how many columns to use? Trial and error?
//    What should it even look like? I need to investigate this further, maybe
//    read the source code of ls
// function for outputting a single file in -l

// prints info on each file specified by the strings contained in the vector
// "files"
void print_files(const vector<string>& files)
{
   cout << "printing files: ";
   for (unsigned i = 0; i < files.size(); ++i)
   {
      cout << files.at(i) << " ";
   }
   cout << endl;
}

// prints the contents of a directory only
void print_contents(const string& dir)
{
   cout << "printing contents of " << dir << ": ";
   /* Printing function */
   // *** You will need columns whether -l is present or not
   // If it is not a directory, print an error message.
   // Otherwise, build a list containing the *stats* of the files it contains (remember to
   //    include .files for the -a flag, and keep a running tally of sizes for -l)
   // * if the stat struct does not contain the name, maybe only make a list of names
   // Sort the list alphabetically
   // If -l
      // make it 1 file per line
   // Else print names in columns
   // Print a newline
   // If -R, run through the list and call the printing function on every
   // directory. TODO: make sure the order in which subdirectories are printed is
   // correct
   // Do not call on . and .., TODO what about other links?
   cout << "\n";
}

// prints the contents of a directory
void print_directory(const string& dir)
{
   cout << dir << ":\n";
   print_contents(dir);
}

int main(int argc, char** argv)
{
   /* Process flags */

   /* TODO REMOVE
   cout << "arguments before getopt: ";
   PRINTARR(argv, argc)
   */

   // Set the proper booleans based on the flags, probably use getopts.
   int flag;
   while (-1 != (flag = getopt(argc, argv, "Ral")))
   {
      switch (flag) {
         case 'R':
            recursive = true;
            break;
         case 'a':
            show_hidden = true;
            break;
         case 'l':
            more_info = true;
            break;
         default:
            cout << flag << " is not a valid flag.\n";
      }
   }

   /* Remove flag arguments */

   /* TODO REMOVE
   cout << "arguments after getopts: ";
   PRINTARR(argv, argc)
   */

   argc -= optind; // set number of arguments to the number of non-flag arguments
   argv += optind; // getopt rearranges all non-flag arguments to the end of argv, so the
                   // following line points argv at the first of these arguments
   // if no directories were given by the user to search, set the directories to
   // search to be "." only.

   /* Process non-flag arguments */ 
   vector<string> dirs;
   vector<string> files;
   // if user didn't specify files/directories to list, just check "." 
   if (argc == 0)
   {
      dirs.push_back(string("."));   
   }
   // check each argument in argv to check if it exists.
   // If it does, push it into the appropriate vector. 
   for (int i = 0; i < argc; ++i)
   {
      struct stat s;
      if (-1 == stat(argv[i], &s))
      {
         perror("cannot access file"); // TODO more descriptive
         continue;
      }
      if (s.st_mode & S_IFDIR)
      {
         dirs.push_back(argv[i]);
      } else {
         files.push_back(argv[i]);
      }
   }

   // sort directories and files alphabetically
   sort(dirs.begin(), dirs.end());
   sort(files.begin(), files.end());

   /* TODO REMOVE
   cout << "dirs and files after sorting: ";
   for (unsigned i = 0; i < dirs.size(); ++i)
      cout << dirs.at(i) << " ";
   cout << endl;
   for (unsigned i = 0; i < files.size(); ++i)
      cout << files.at(i) << " ";
   cout << endl;
   */
   
   /* Print output */

   // special case where you don't want to print the directory's name
   if (dirs.size() == 1 && files.size() == 0 && !recursive)
   {
      print_contents(dirs.at(0));
   } else {
      // files first
      if (files.size() > 0)
      {
         print_files(files);
         if (dirs.size() > 0) cout << "\n";
      }
      // then directories
      if (dirs.size() > 0)
      {
         for (unsigned i = 0; i < dirs.size() - 1; ++i)
         {
            print_directory(dirs.at(i)); // call the printing function
            cout << "\n"; // put an extra newline between multiple printed directories
         }
         print_directory(dirs.at(dirs.size() - 1));
      }
   }

   return 0;
}
