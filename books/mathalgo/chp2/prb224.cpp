#include <iostream>
using namespace std;

int32_t N, A[129];

int32_t main()
{
  int32_t sum = 0;
  cout << "N(1<=N<=128)を代入"<<endl;
  cin >> N;
  
  for(uint32_t i = 1u; i <=N; i++)
  {
    cin >> A[i];
  }

  for(uint32_t i = 1u; i <=N; i++)
  {
    sum += A[i];
  }

  cout << "sum % 100=" << sum % 100 << endl;

  return 0;
}
