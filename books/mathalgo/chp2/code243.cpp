#include <iostream>
using namespace std;

int32_t N, S;
long long Ans = 0;

int32_t main()
{
  //入力
  cin >> N >> S;

  for(int32_t i = 1; i <= N; i++)
  {
    for(int32_t j = 1; j <= N; j++)
    {
      if(i+j <= S)
      {
        Ans += 1;
      }
    }
  }

  //出力
  cout << Ans << endl;

  return 0;
}
