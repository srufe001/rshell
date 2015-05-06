#include<iostream>
#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<cstring>

using namespace std;

int main(int argc, char** argv)
{
   int flag;
   while(-1 != (flag = getopt(argc, argv, "")))
   {
   }
   argc -= optind;
   argv += optind;

   if (argc != 2)
   {
      cout << "you must specify exactly two files\n";
   }

   struct stat s;

   if (-1 == stat(argv[0], &s))
   {
      perror(argv[0]);
      return 0;
   }

   int returnValue = access(argv[1], F_OK);
   if (returnValue == 0)
   {
      if (-1 == stat(argv[1], &s))
      {
         perror(argv[1]);
         return 0;
      }

      // it exists, check if it's a directory      
      if (s.st_mode & S_IFDIR)
      {
         string newpath;
         newpath += argv[1];
         newpath += "/";
         newpath += argv[0];

         if (-1 == link(argv[0], newpath.c_str()))
         {
            if (errno == EEXIST)
            {
               cout << newpath << " already exists\n";
            } else {
               perror("link");
            }
         } else {
            unlink(argv[0]);
         }
      } else {
         cout << argv[1] << " already exists\n";
         return 0;
      }
   } else if (returnValue == -1)
   {
      // it does not exist, just rename
      if (-1 == link(argv[0], argv[1]))
      {
         perror("link");
      } else {
         unlink(argv[0]);
      }
   }
}
