
#include <iostream>
#include <sstream>
#include <assert.h>
#include "isa.h"
using namespace std;

int testf(int id, char* tarr[], int tarr_size, int ass_out, bool mute_cout, ostringstream *local_cout){
    static auto cout_buff = cout.rdbuf();
    int retv = 0;
    cout << "\ntest " << id << ": ";
    for (int i = 0; i < tarr_size; i++){
        cout << tarr[i] << " ";
    }
    cout << "\n";
    if (mute_cout){
        cout.rdbuf(local_cout->rdbuf());
    }
    retv = (isa_main(tarr_size, tarr) == ass_out) ? 0 : 1;
    if (mute_cout){
        cout.rdbuf(cout_buff);
    }
    if (retv == 0){
        cout << "test " << id << " OK\n";
    }
    else {
        cout << "test " << id << " fail\n";
    }
    return retv;
}

int main(){
  ostringstream local_cout;
  auto cout_buff = cout.rdbuf();

  char *tarr1[1] = {(char *)"fname"};
  //testf(1, tarr1, 1, 1, true, &local_cout);

  char *tarr2[4] = {(char *)"fname", (char *)"-f", (char *)"ff.txt", (char *)"-u"};
  //testf(2, tarr2, 4, 0, true, &local_cout);

  char *tarr3[3] = {(char *)"fname", (char *)"-f", (char *)"-u"};
  //testf(3, tarr3, 3, 2, true, &local_cout);

  char *tarr4[3] = {(char *)"fname", (char *)"-f", (char *)"empty.txt"};
  //testf(4, tarr4, 3, 5, true, &local_cout);

  char *tarr5[2] = {(char *)"fname", (char *)"https://what-if.xkcd.com/feed.atom"};
  //testf(5, tarr5, 2, 0, false, &local_cout);

  char *tarr6[2] = {(char *)"fname", (char *)"https://xkcd.com/atom.xml"};
  testf(6, tarr6, 2, 0, false, &local_cout);

}
