#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <map>

using namespace std;

int n;
map<int, int> f;

int main() {
  cin >> n;
  while (n--) {
    int key, value;
    cin >> key >> value;
    f[key] = value;
  }
  size_t acc = 0;
  int x;
  while (cin >> x) acc += f[x];
  cout << acc << endl;
  return 0;
}
