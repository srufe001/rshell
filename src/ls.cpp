//TODO: look up how ls prints in columns
//Do I have to delete stuff like the stat object

#include<iostream>
#include<unistd.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<vector>
#include<algorithm>
#include<string>
#include<cstring>

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
// "files", also prints subdirectories recursively if -R flag is enabled
void print_files(vector<string>& files)
{
   sort(files.begin(), files.end());

   unsigned term_width = 80;
   // -l is not used
   if (!more_info)
   {
      string output;
      for (unsigned i = 0; i < files.size(); ++i)
      {
         if (output.size() == 0)
         {
            output += files.at(i);
         } else if (output.size() + 2 + files.at(i).size() > term_width) {
            cout << output << '\n';
            output = files.at(i);
         } else {
            output += "  " + files.at(i);
         }
      }
      cout << output << endl;
   } else { // -l was used
      // get stat objects for everything
      vector<struct stat> stats;
      for (unsigned i = 0; i < files.size(); ++i)
      {
         struct stat temp;
         if (-1 == stat(files.at(i).c_str(), &temp))
         {
            perror("unable to open file");   // TODO make more descriptive
            continue;
         }
         stats.push_back(temp);
      }
      /*
      int total_links = 0;
      int max_user_name_length = 0;
      int max_group_name_length = 0;
      int max_linked_files_length = 0;
      int max_bit_size_length = 0;
      int max_date+length = 0;
      for (int i = 0; 
      */
   }
   // If -R, run through the list and call the printing function on every
   // DONT FORGET NOT TO RECURSE INTO . AND ..
   // directory. TODO: make sure the order in which subdirectories are printed is
   // correct
   // Do not call on . and .., TODO what about other links?
   // DONT FORGET EXTRA NEWLINES
}

// makes a vector of files and calls print_files on it
void print_contents(const string& dir)
{
   DIR * dirstream; 
   if (NULL == (dirstream = opendir(dir.c_str())))
   {
      perror("unable to open directory"); // TODO make more descriptive 
      return;
   }

   // create a list of the files contained in this directory
   vector<string> files;

   struct dirent * dent;
   while (NULL != (dent = readdir(dirstream)))
   {
      if (!show_hidden && strlen(dent->d_name) > 0 && dent->d_name[0] == '.')
         continue;
      files.push_back(string(dent->d_name));
   }
   // TODO you will have to check if errno got set an call perror
   
   if (-1 == closedir(dirstream))
   {
      perror("failed to close directory"); // TODO make more descriptive
      return;
   }

   // print files. This also alphabetizes and does recursive printing if -R flag is used
   print_files(files);
}

// prints the directory
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
         continue;   // a file that doesn't exist shouldn't be used
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
