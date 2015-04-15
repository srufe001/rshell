#include<iostream>
#include<string>
#include<cstdlib>
#include<boost/tokenizer.hpp>

using namespace std;
using namespace boost;

int main()
{
   while (true)
   {
      // cout prompt
      cout << "$ ";
      // get user input
      string userin;
      getline(cin, userin);
      cout << "user inputted: " << userin << endl;
      // !!!divide up user input
      tokenizer<> tok(userin);
      for (auto it=tok.begin(); it != tok.end(); ++it)
      {
         cout << *it << endl;
      }
      //slice off comment (everything after and including '#')
      // exec the individual commands
   }

   return 0;
}
