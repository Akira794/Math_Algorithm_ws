#include <iostream>
using namespace std;

int32_t main()
{
  long long N;
  long long Ans = 1;
  cin >> N;

  for(int32_t i = 2; i <= N; i++)
  {
    Ans *= i;
  }
  
  cout << Ans << endl;

  return 0;
}
