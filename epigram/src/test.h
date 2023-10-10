#include <doctest.h>
#include <rapidcheck.h>

#include <utility>

namespace rc {
template <typename Testable_>
void prop (const std::string &description, Testable_ &&testable) {
  using namespace detail;
  DOCTEST_SUBCASE(description.c_str()) {
    const auto result = checkTestable(std::forward<Testable_>(testable));

    if (result.template is<SuccessResult> ()) {
      const auto success = result.template get<SuccessResult> ();
      if (! success.distribution.empty ()) {
        std::cout << "- " << description << '\n';
        printResultMessage (result, std::cout);
        std::cout << std::endl;
      }
    }
    else {
      std::ostringstream ss;
      printResultMessage (result, ss);
      DOCTEST_FAIL_CHECK (ss.str() << "\n");
    }
  }
}

template<>
struct Arbitrary<std::byte> {
  static Gen<std::byte> arbitrary() {
    return gen::map<std::uint8_t>([](std::uint8_t x) {
          return static_cast<std::byte>(x);
        });
  }
};

}
