#include <iostream>

using namespace std;

int n;
int m;

int main() {
  cin >> n;
  m = 3*n;
  cout << 4 + n + 1 + 2*m + n << endl;
  cout << 0 << ' ' << 0 << endl;
  cout << 1 << ' ' << n-1 << endl;
  cout << 2 << ' ' << n << endl;
  cout << 3 << ' ' << m << endl;
  for (int i = 0; i <= n; i++) {
    cout << 4+i << ' ' << 3*i << endl;
  }
  for (int i = 0; i < n; i++) {
    cout << 4 + n + 1 + 3*i << ' ' << (i+1)%n << endl;
    cout << 4 + n + 2 + 3*i << ' ' << (i+2)%n << endl;
    cout << 4 + n + 3 + 3*i << ' ' << (i+3)%n << endl;
  }
  for (int i = 0; i < n; i++) {
    cout << 4 + n + m + 1 + 3*i << ' ' << 1 << endl;
    cout << 4 + n + m + 2 + 3*i << ' ' << 1 << endl;
    cout << 4 + n + m + 3 + 3*i << ' ' << 1 << endl;
  }
  for (int i = 0; i < n; i++) {
    cout << 900 + i << ' '  << 200000000 << endl;
  }
  return 0;
}
