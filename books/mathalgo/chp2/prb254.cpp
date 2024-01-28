#include <iostream>
using namespace std;

bool isprime(int32_t x)
{
  bool ret = true;

  for(int32_t i = 2; i <= x - 1; i++)
  {
    //xをiで割った余りが0のときは割り切れる
    if( x % i == 0)
    {
      ret = false;
    }
  }

  return ret;
}

int32_t main()
{
  int32_t N, Ans = 0;
  cin >> N;

  for(int32_t i = 2; i <= N; i++)
  {
    if(isprime(i) == true)
    {
      cout << i << endl;
    }
  }

  return 0;
}
