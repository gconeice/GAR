#include <iostream>
#include <cmath>


constexpr std::uint64_t log2(std::uint64_t n) {
  std::uint64_t out = 0;
  n = n * 2 - 1;
  while (n > 1) {
    ++out;
    n >>= 1;
  }
  return out;
}


std::size_t q(std::size_t n) {
  std::cout << n << '\n';
  if (n < 4) {
    return 2;
  } else {
    const std::size_t r = sqrt(n);
    std::cout << "  " << r << '\n';
    return q(2*r-1) + q(r - 1);
  }
}


int main(int argc, char** argv) {
  std::size_t n = atoi(argv[1]);
  std::cout << q(n) << '\n';
}
