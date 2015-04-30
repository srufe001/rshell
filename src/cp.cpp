#include<iostream>
#include<istream>
#include<istream>
#include<cstdlib>
#include<fstream>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<stdio.h>
#include "Timer.h"

using namespace std;

void process1(const char * src, const char * dst)
{
   ifstream myBuff (src);
   if (!myBuff.is_open()) 
   {
      cout <<"didn't open" << endl;
   }
   ofstream ofs (dst);
   char c = myBuff.get();
   while(myBuff.good())
   {
      ofs.put(c);
      c = myBuff.get();
   }
   myBuff.close();
   ofs.close();
}

void process2(const char * src, const char * dst)
{
   // open source and destination files
   int fdsrc = open(src, O_RDONLY);
   if (fdsrc == -1) {
      perror("opening source file failed");
      return;
   }
   int fddst = open(dst, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
   if (fddst == -1) {
      perror("opening destination file failed");
      return;
   }

   // read one character at a time from fdsrc, and write each character to fddst
   char buf[1];
   int num_read;
   while((num_read = read(fdsrc, buf, 1)) != 0) 
   {
      if (num_read == -1)
         perror("read failed");
      if (-1 == write(fddst, buf, num_read))
      {
         perror("write failed");
      }
   }

   // close file descriptors
   if (-1 == close(fdsrc))
      perror("unable to close source file");
   if (-1 == close(fddst))
      perror("unable to close destination file");
}

void process3(const char * src, const char * dst)
{
   // open source and destination files
   int fdsrc = open(src, O_RDONLY);
   if (fdsrc == -1) {
      perror("opening source file failed");
      return;
   }
   int fddst = open(dst, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
   if (fddst == -1) {
      perror("opening destination file failed");
      return;
   }

   // read one character at a time from fdsrc, and write each character to fddst
   char buf[BUFSIZ];
   int num_read;
   while((num_read = read(fdsrc, buf, BUFSIZ)) != 0) 
   {
      if (num_read == -1)
         perror("read failed");
      if (-1 == write(fddst, buf, num_read))
      {
         perror("write failed");
      }
   }

   // close file descriptors
   if (-1 == close(fdsrc))
      perror("unable to close source file");
   if (-1 == close(fddst))
      perror("unable to close destination file");
}

struct stat s;

int main(int argc, char ** argv)
{
   if (argc < 3)
   {
      cout << "Not enough arguments\n";
   }
   if (argc == 4)
   {
      // timing stuff
      Timer t1;
      t1.start();
      double time1;
      process1(argv[1], argv[2]);

      cout << "Times for process 1:" << endl;
      t1.elapsedWallclockTime(time1);
      cout << "Wallclock time: " << time1 << endl;
      t1.elapsedUserTime(time1);
      cout << "User time: " << time1 << endl;
      t1.elapsedSystemTime(time1);
      cout << "System time: " << time1 << endl;

      Timer t2;
      t2.start();
      process2(argv[1], argv[2]);

      cout << "Times for process 2:" << endl;
      t2.elapsedWallclockTime(time1);
      cout << "Wallclock time: " << time1 << endl;
      t2.elapsedUserTime(time1);
      cout << "User time: " << time1 << endl;
      t2.elapsedSystemTime(time1);
      cout << "System time: " << time1 << endl;

      Timer t3;
      t3.start();
      process3(argv[1], argv[2]);

      cout << "Times for process 3:" << endl;
      t3.elapsedWallclockTime(time1);
      cout << "Wallclock time: " << time1 << endl;
      t3.elapsedUserTime(time1);
      cout << "User time: " << time1 << endl;
      t3.elapsedSystemTime(time1);
      cout << "System time: " << time1 << endl;

      return 0;
   }
  
   stat(argv[1], &s); 
   if (s.st_mode & S_IFDIR)
   {
      cout << "cannot copy, " << argv[1] << " is a directory." << endl;
      return 1;
   }

   if (-1 != stat(argv[2], &s))
   {
      cout << "cannot copy, " << argv[2] << " already exists." << endl;
      return 1;
   }
   
   
   process3(argv[1], argv[2]); 
}
