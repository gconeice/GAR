#include <iostream>
#include <algorithm>


constexpr std::uint64_t sigma = 80;


constexpr std::uint64_t log2(std::uint64_t n) {
  std::uint64_t out = 0;
  n = n * 2 - 1;
  while (n > 1) {
    ++out;
    n >>= 1;
  }
  return out;
}


std::uint64_t disperse(std::uint64_t n, std::uint64_t k) {
  /* if (n <= n/2 + 8*sqrt(n) || k < 2*n) { */
  if (k < n + 16*sqrt(n)) {
    return k * log2(k);
  } else {
    return 2*disperse(n/2 + 8*sqrt(n), k/2) + 16*sqrt(n);
  }
}


int main() {
  for (std::uint64_t logn = 0; logn < 40; ++logn) {
    const std::uint64_t n = 1 << logn;
    const std::uint64_t k = n*logn;
    std::cout << disperse(n, k) << '\n';
  }
}
