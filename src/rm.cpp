#include<iostream>
#include<unistd.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<cstring>

using namespace std;

bool removeDirectories = false;

void rem(const char* path)
{
   struct stat s;
   if (-1 == stat(path, &s))
   {
      perror(path);
      return;
   }
   // check if its a directory
   if (s.st_mode & S_IFDIR)
   {
      if (removeDirectories)
      {
         DIR * dirstream;
         if (NULL == (dirstream = opendir(path)))
         {
            perror("opendir");
            return;
         }
         struct dirent * dent;
         while (NULL != (dent = readdir(dirstream)))
         {
            if (!(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")))
            {
               string dirname;
               dirname += path;
               dirname += "/";
               dirname += dent->d_name;
               rem(dirname.c_str());
            }
         }
         closedir(dirstream);
         if (-1 == rmdir(path))
         {
            perror("rmdir");
         }
      } else {
         cout << path << " is a directory. Use the -r flag to remove directories\n";
      }
   } else if (-1 == unlink(path)) {
      perror("unlink");
   }
}

int main(int argc, char** argv)
{
   int flag;
   while(-1 != (flag = getopt(argc, argv, "r")))
   {
      if (flag == 'r')
         removeDirectories = true;
   }
   argc -= optind;
   argv += optind;

   for (int i = 0; i < argc; ++i)
   {
      rem(argv[i]);
   }
}
