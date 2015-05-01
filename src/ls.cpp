//TODO: look up how ls prints in columns
//Do I have to delete stuff like the stat object

#include<iostream>
#include<unistd.h>
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
   cout << "printing files" << endl;
   for (unsigned i = 0; i < files.size(); ++i)
   {
      cout << files.at(i) << " ";
   }
   cout << endl;
}

// prints the contents of a directory
void print_contents(const string& dir)
{
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
   
}

int main(int argc, char** argv)
{
   /* Process flags */
   cout << "arguments before getopt: ";
   PRINTARR(argv, argc)
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

   /* Process non-flag arguments */
   cout << "arguments after getopts: ";
   PRINTARR(argv, argc)
   argc -= optind; // set number of arguments to the number of non-flag arguments
   argv += optind; // getopt rearranges all non-flag arguments to the end of argv, so the
                   // following line points argv at the first of these arguments
   // if no directories were given by the user to search, set the directories to
   // search to be "." only.
   
   // fill a vector with directories to search
   vector<string> dirs;
   if (argc == 0)
   {
      dirs.push_back(string(".")); 
   } else {
      for (int i = 0; i < argc; ++i)
         dirs.push_back(string(argv[i]));
   }
   // sort directories alphabetically
   sort(dirs.begin(), dirs.end());
   for (unsigned i = 0; i < dirs.size(); ++i)
      cout << dirs.at(i) << " ";
   cout << endl;
   
   /* Print output */
   // For each file in the list, print errors for directories
   for (int i = 0; i < argc; ++i)
      print_contents(dirs.at(i)); // call the printing function

   return 0;
}
