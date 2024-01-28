#include <iostream>
using namespace std;

int32_t main()
{
  int32_t N, X, Y;

  cin >> N >> X >> Y;

  //計算
  int32_t count = 0;
  for(int32_t i = 1; i <= N; i++)
  {
    if(i % X == 0 || i % Y == 0)
    {
      count++;
    }
  }

  cout << count << endl;

  return 0;
}
