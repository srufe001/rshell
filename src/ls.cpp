#include<iostream>
#include<iomanip>
#include<unistd.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<vector>
#include<algorithm>
#include<string>
#include<cstring>
#include<time.h>
#include<pwd.h>
#include<grp.h>
#include<cerrno>

using namespace std;

#define PRINTARR(arr, num_elements) { for (int i = 0; i < num_elements; ++i) \
                                         cout << arr[i] << " "; \
                                      cout << endl; } \

bool show_hidden = false;
bool recursive = false;
bool more_info = false;

void print_directory(const string&);

unsigned numDigits(int n)
{
   unsigned digits = 0;
   while (n) {
      n /= 10;
      ++digits;
   }
   return digits;
}

string fileName(string& file, struct stat& s)
{
   string filename;

   if (file.size() > 0 && file.at(0) == '.')
      filename += "\x1b[47m";

   if (s.st_mode & S_IFREG) {
      if (s.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))  // file is executable
         filename += "\x1b[32m" + file + "\x1b[0m" + '*';
      else
         filename += file;
   } else if (s.st_mode & S_IFDIR) {
      filename += "\x1b[34m" + file + "\x1b[0m" + '/';
   } else if (s.st_mode & S_IFCHR) {
      filename += file;
   } else if (s.st_mode & S_IFBLK) {
      filename += file;
   } else if (s.st_mode & S_IFIFO) {
      filename += file;
   } else if (s.st_mode & S_IFLNK) {
      filename += file + '@';
   } else if (s.st_mode & S_IFSOCK) {
      filename += file;
   }
   return filename + "\x1b[0m";
}

void print_files(vector<string>& files, string dirname = ".", bool printBlockSize = true)
{
   if (files.size() == 0) return;
   sort(files.begin(), files.end());

   // get stat objects for everything
   vector<struct stat> stats;
   vector<bool> stat_exists;
   for (unsigned i = 0; i < files.size(); ++i)
   {
      struct stat temp;
      string filename = dirname + "/" + files.at(i);
      if (-1 == stat(filename.c_str(), &temp))
      {
         string errorMessage = "unable to open file " + files.at(i);
         perror(errorMessage.c_str());
         stat_exists.push_back(false);
         continue;
      }
      stats.push_back(temp);
      stat_exists.push_back(true);
   }

   unsigned term_width = 80;
   if (!more_info)
   {
      char col_sep[] = "  ";
      unsigned col_sep_length = sizeof(col_sep) / sizeof(*col_sep);
      unsigned cols = 2;
      unsigned rows;
      // calculate rows, cols
      while (true)
      {
         rows = (files.size() + cols - 1) / cols;  // round up
         // check if cols is too high
         if (cols > files.size())
         {
            --cols;
            rows = (files.size() + cols - 1) / cols; // round up
            break;
         }
         // calculate total length of output with this number of cols
         unsigned row_length = 0;
         //check each column
         for (unsigned i = 0; i < cols; ++i)
         {
            unsigned longest_filename = 0;
            for (unsigned j = 0; j < rows; ++j)
            {
               unsigned index = i * rows + j;
               if (index >= files.size())
                  break;
               if (files.at(index).size() > longest_filename)
                  longest_filename = files.at(index).size();
            }
            row_length += longest_filename + col_sep_length;
         }
         row_length -= col_sep_length;  // make up for the extra spaces added

         if (row_length > term_width)
         {
            --cols;
            rows = (files.size() + cols - 1) / cols;  // round up
            break;
         }
         ++cols;
      }
      // remove unnecessary columns
      cols = (files.size() + rows - 1) / rows; // round up
      // calculate column widths
      vector<unsigned> colwidths;
      for (unsigned i = 0; i < cols; ++i)
      {
         unsigned longest_filename = 0;
         for (unsigned j = 0; j < rows; ++j)
         {
            unsigned index = i * rows + j;
            if (index >= files.size())
               break;
            if (files.at(index).size() > longest_filename)
               longest_filename = files.at(index).size();
         }
         colwidths.push_back(longest_filename);
      }

      //print each row
      unsigned num_full_rows = rows - (rows * cols - files.size());  // number of rows that will have cols items
      for (unsigned i = 0; i < rows; ++i)
      {
         // print each column of the row
         unsigned j;
         // decide how many cols to print in this row
         unsigned num_to_print = (i >= num_full_rows) ? cols - 1 : cols;
         for (j = 0; j < num_to_print - 1; ++j) //min(cols, static_cast<unsigned>(files.size() - cols * i)) - 1; ++j)
         {
            if (j >= colwidths.size()) cout << "fail 1" << endl;
            if (i + rows * j >= files.size()) cout << "fail 2" << endl;

            cout << setw(colwidths.at(j)) << left << files.at(i + rows * j)
                 << col_sep;
         }
         if (j >= colwidths.size()) cout << "fail 3" << endl;
         if (i + rows * j >= files.size()) cout << "fail 4" << endl;
         cout << setw(colwidths.at(j)) << left << files.at(i + rows * j) <<
         '\n';
      }
   } else {
      // calculate unknown column widths and number of hard links
      unsigned total_blocks = 0;
      unsigned max_user_name_length = 0;
      unsigned max_group_name_length = 0;
      unsigned max_linked_files_length = 0;
      unsigned max_byte_size_length = 0;
      //int max_date_length = 0;
      for (unsigned i = 0; i < files.size(); ++i)
      {
         if (stat_exists.at(i))
         {
            struct stat & filestat = stats.at(i);

            total_blocks += filestat.st_blocks;

            if (filestat.st_size > max_byte_size_length)
               max_byte_size_length = filestat.st_size;

            if (filestat.st_nlink > max_linked_files_length)
               max_linked_files_length = filestat.st_nlink;

            struct passwd * username;
            if (NULL == (username = getpwuid(filestat.st_uid)))
            {
               perror("unable to get username");
            } else {
               if (strlen(username->pw_name) > max_user_name_length)
                  max_user_name_length = strlen(username->pw_name);
            }

            struct group * groupname;
            if (NULL == (groupname = getgrgid(filestat.st_gid)))
            {
               perror("unable to get groupname");
            } else {
               if (strlen(groupname->gr_name) > max_group_name_length)
                  max_group_name_length = strlen(groupname->gr_name);
            }
         }
      }
      // calculate digits of max_bit_size_length and max_linked_files_length
      max_byte_size_length = numDigits(max_byte_size_length);
      max_linked_files_length = numDigits(max_linked_files_length);
      // total_blocks is in 512 byte blocks, this shouldbe 1k bype blocks
      // TODO check if this gives the right output
      total_blocks /= 2;

      // output block size
      if (printBlockSize)
         cout << "total: " << total_blocks << '\n';
      
      // output data
      for (unsigned i = 0; i < files.size(); ++i)
      {
         if (stat_exists.at(i))
         {
            struct stat & filestat = stats.at(i);

            // print filetype
            if (filestat.st_mode & S_IFREG) {
               cout << '-';
            } else if (filestat.st_mode & S_IFDIR) {
               cout << 'd';
            } else if (filestat.st_mode & S_IFCHR) {
               cout << 'c';
            } else if (filestat.st_mode & S_IFBLK) {
               cout << 'b';
            } else if (filestat.st_mode & S_IFIFO) {
               cout << 'p';
            } else if (filestat.st_mode & S_IFLNK) {
               cout << 'l';
            } else if (filestat.st_mode & S_IFSOCK) {
               cout << 's';
            }

            // print permissions
            cout << ((filestat.st_mode & S_IRUSR) ? 'r' : '-')
                 << ((filestat.st_mode & S_IWUSR) ? 'w' : '-')
                 << ((filestat.st_mode & S_IXUSR) ? 'x' : '-')
                 << ((filestat.st_mode & S_IRGRP) ? 'w' : '-')
                 << ((filestat.st_mode & S_IWGRP) ? 'w' : '-')
                 << ((filestat.st_mode & S_IXGRP) ? 'w' : '-')
                 << ((filestat.st_mode & S_IROTH) ? 'w' : '-')
                 << ((filestat.st_mode & S_IWOTH) ? 'w' : '-')
                 << ((filestat.st_mode & S_IXOTH) ? 'w' : '-');

            cout << ' ';

            // print number of hard links pointing to this file
            cout << setw(max_linked_files_length) << right << filestat.st_nlink;

            cout << ' ';

            // print owner's username
            struct passwd * username;
            if (NULL == (username = getpwuid(filestat.st_uid)))
            {
               string errorMessage = "unable to get username for uid " + filestat.st_uid;
               perror(errorMessage.c_str());
            } else {
               cout << setw(max_user_name_length) << right << username->pw_name;
            }

            cout << ' ';

            // print owner's group
            struct group * groupname;
            if (NULL == (groupname = getgrgid(filestat.st_gid)))
            {
               string errorMessage = "unable to get groupname for guid " + filestat.st_gid;
               perror(errorMessage.c_str());
            } else {
               cout << setw(max_group_name_length) << right << groupname->gr_name;
            }

            cout << ' ';

            // print byte size
            cout << setw(max_byte_size_length) << right << filestat.st_size;
            
            cout << ' ';

            // print the date of last status change
            time_t time = filestat.st_ctime;
            struct tm* timeptr = localtime(&time);
            if (timeptr == NULL)
               perror("failed to get ctime");
            char timestr[100];
            strftime(timestr,sizeof(timestr),"%b %e %H:%M", timeptr);
            cout << timestr;

            cout << ' ';

            // print file name
            cout << fileName(files.at(i), stats.at(i));
            cout << '\n';
         }
      }
   }
   // If -R, run through the list and call the printing function on every
   if (recursive)
   {
      for (unsigned i = 0; i < files.size(); ++i)
      {
         if (stat_exists.at(i) && (stats.at(i).st_mode & S_IFDIR) && files.at(i) != "." && files.at(i) != "..")
         {
            cout << '\n';
            print_directory(dirname + '/' + files.at(i));
         }
      }
   }
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
      string errorMessage = "unable to open directory " + dir;
      perror(errorMessage.c_str());
      return;
   }

   // create a list of the files contained in this directory
   vector<string> files;

   int tmp_old_errno = errno;
   struct dirent * dent;
   while (NULL != (dent = readdir(dirstream)))
   {
      if (!show_hidden && strlen(dent->d_name) > 0 && dent->d_name[0] == '.')
         continue;
      files.push_back(dent->d_name);
   }
   if (errno != tmp_old_errno)
   {
      string errorMessage = "unable to read from file " + dir;
      perror(errorMessage.c_str());
   }
   
   if (-1 == closedir(dirstream))
   {
      string errorMessage = "failed to close directory " + dir;
      perror(errorMessage.c_str());
      return;
   }

   // print files. This also alphabetizes and does recursive printing if -R flag is used
   print_files(files, dir);
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
   
   /* Print output */

   // special case where you don't want to print the directory's name
   if (dirs.size() == 1 && files.size() == 0 && !recursive)
   {
      print_contents(dirs.at(0));
   } else {
      // files first
      if (files.size() > 0)
      {
         print_files(files, ".", false);
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
