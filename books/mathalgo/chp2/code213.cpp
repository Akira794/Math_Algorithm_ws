#include <iostream>
using namespace std;

#define ArrayMax 64

int32_t main()
{
  uint32_t N;
  uint32_t A[ArrayMax];
  uint32_t Sum = 0u;

  cout << "配列要素数を決める" << endl;
  cin >> N;

  for (uint32_t i = 1u; i <= N; i++)
  {
    cout <<"N=" << i << "<<<";
    cin >> A[i];
    Sum += A[i];
  }
  cout << "Sum: " << Sum << endl;
  return 0;
}
