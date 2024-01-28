#include <iostream>
#include <string>
using namespace std;

uint32_t N;
string RetStr;

int32_t main()
{
  uint32_t dec = 0u;
  cout << "10進数を2進数に変換"<<endl;
  cin >> N;

  dec = N;
  while(N >= 1)
  {
    //N % 2はMを2で割った余り(例:N=13の場合 1)
    //N / 2はNを2で割った値の整数部分(例:N=13 の場合6)

    if(N % 2 == 0) RetStr = "0" + RetStr;
    if(N % 2 == 1) RetStr = "1" + RetStr;
    N = N / 2;

  }
  cout << "DEC:" << dec <<endl;
  cout << "BIN:"<< RetStr <<endl;
  return 0;
}
