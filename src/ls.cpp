//TODO: look up how ls prints in columns

#include<iostream>

using namespace std;

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

int main(int argc, char** argv)
{
   /* Process flags and arguments*/
   // Set the proper booleans based on the flags, probably use getopts.
   // Fill a list of directories to search (not including recursive ones).
   // If this list is empty after all arguments have been processed, add "." to it.
   
   /* Printing output */
   // For each file in the list
      // call the printing function

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
   


   return 0;
}
