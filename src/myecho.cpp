#include<iostream>
#include<unistd.h>
#include<cstring>
#include<cstdlib>
#include<stdio.h>

using namespace std;

int main(int argc, char** argv)
{
   if (argc < 3)
   {
      write(1, "usage", 5);
      return 1;
   }
   for (int i = 2; i < argc; ++i)
   {
      if (-1 == write(atoi(argv[1]), argv[i], strlen(argv[i])))
      {
         perror("myecho write");
         return 1;
      }
      if (i != argc - 1)
      {
         if (-1 == write(atoi(argv[1]), " ", 1))
         {
            perror("myecho write");
            return 1;
         }
      }
   }
   if (-1 == write(atoi(argv[1]), "\n", 1))
   {
      perror("myecho write");
      return 1;
   }
   return 0;
}
