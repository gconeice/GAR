#include <simplestack.h>
#include <util.h>
#include <resource.h>


template <Mode m>
std::vector<Garbled::Bit<m>> epop(
    std::size_t w, const Garbled::Bit<m>& p, std::span<Garbled::Bit<m>> buff) {

  assert(buff.size() % w == 0);
  const auto n = buff.size() / w;

  std::vector<Garbled::Bit<m>> temp(w);
  for (auto& t: temp) { t = constant<m>(0); }

  for (std::size_t i = 0; i < n; ++i) {
    auto slot = buff.subspan((n - i - 1)*w, w);
    eswap(p, slot, std::span { temp });
  }

  return temp;
}

template std::vector<Garbled::Bit<Mode::G>> epop(
    std::size_t, const Garbled::Bit<Mode::G>&, std::span<Garbled::Bit<Mode::G>>);
template std::vector<Garbled::Bit<Mode::E>> epop(
    std::size_t, const Garbled::Bit<Mode::E>&, std::span<Garbled::Bit<Mode::E>>);
template std::vector<Garbled::Bit<Mode::S>> epop(
    std::size_t, const Garbled::Bit<Mode::S>&, std::span<Garbled::Bit<Mode::S>>);


constexpr std::size_t level_width = 3;
constexpr std::size_t vacancy_width = level_width + 1;

template <Mode m>
SimpleStack<m>::SimpleStack(std::size_t w, std::span<const Garbled::Bit<m>> init)
  : w(w), timer(0) {

  assert(init.size() % w == 0);
  const auto n = init.size() / w;

  // A stack with l levels can store 3*(2^l - 1) words
  // The following accordingly calculates l from n words
  const auto levels = log2(((n + 2) / 3) + 1);

  content.resize(3*w*((1 << levels) - 1));
  std::copy(init.begin(), init.end(), content.begin());

  // set up the vacancy metadata such that each level has zero vacancies
  vacancies.resize(vacancy_width * levels);
  for (std::size_t i = 0; i < levels; ++i) {
    vacancies[vacancy_width*i] = constant<m>(1);
  }
}


template SimpleStack<Mode::G>::SimpleStack(std::size_t, std::span<const Garbled::Bit<Mode::G>>);
template SimpleStack<Mode::E>::SimpleStack(std::size_t, std::span<const Garbled::Bit<Mode::E>>);
template SimpleStack<Mode::S>::SimpleStack(std::size_t, std::span<const Garbled::Bit<Mode::S>>);

template <Mode m>
std::vector<Garbled::Bit<m>> pop_stack(
    std::size_t w,
    std::size_t timer,
    const Garbled::Bit<m>& p,
    std::span<Garbled::Bit<m>> content,
    std::span<Garbled::Bit<m>> vacancies) {

  auto level = content.subspan(0, 3*w);

  // conditionally pop the overall output
  const auto output = epop(w, p, level);

  // update the vacancy counter
  // TODO there should be a faster method for computing these
  vacancies[3] = eand_full(p, vacancies[2]) ^ eand_full(~p, vacancies[3]);
  vacancies[2] = eand_full(p, vacancies[1]) ^ eand_full(~p, vacancies[2]);
  vacancies[1] = eand_full(p, vacancies[0]) ^ eand_full(~p, vacancies[1]);
  vacancies[0] = vacancies[1] ^ vacancies[2] ^ vacancies[3] ^ constant<m>(1);

  if ((timer & 1) && content.size() > 3*w) {
    // if there are at least two vacancies, pop the next level to fetch two more elements
    const auto recp = vacancies[2] ^ vacancies[3];
    const auto back_fill = pop_stack(
        w*2,
        timer >> 1,
        recp,
        content.subspan(3*w),
        vacancies.subspan(vacancy_width));

    // load in the two popped elements into the back of this level
    level.subspan(w) ^= std::span { back_fill };

    // if there were two or three vacancies, vacancies have decreased by two
    vacancies[0] ^= vacancies[2];
    vacancies[1] ^= vacancies[3];
    vacancies[2] = constant<m>(0);
    vacancies[3] = constant<m>(0);

    // if there is exactly one vacancy and we recursively popped, then we need to
    // slide the level forward one word
    // this will ensure that the level is front-filled
    // note that if this indeed pops, the first word is all zeros, so we need
    // not pop that front word, and instead write to it
    const auto new_first_word = epop(w, eand_full(recp, vacancies[1]), level.subspan(w));
    level.subspan(0, w) ^= std::span { new_first_word };
  }

  return output;
}



template <Mode m>
std::vector<Garbled::Bit<m>> SimpleStack<m>::pop(const Garbled::Bit<m>& p) {
  const auto out = pop_stack(w, timer, p, std::span { content }, std::span { vacancies });
  ++timer;
  return out;
}

template std::vector<Garbled::Bit<Mode::G>>
SimpleStack<Mode::G>::pop(const Garbled::Bit<Mode::G>&);
template std::vector<Garbled::Bit<Mode::E>>
SimpleStack<Mode::E>::pop(const Garbled::Bit<Mode::E>&);
template std::vector<Garbled::Bit<Mode::S>>
SimpleStack<Mode::S>::pop(const Garbled::Bit<Mode::S>&);


std::vector<bool> stack_spec(std::size_t w, const std::vector<bool>& content, const std::vector<bool>& query) {
  std::vector<bool> out(query.size() * w);
  std::size_t ix = 0;
  for (std::size_t i = 0; i < query.size(); ++i) {
    for (std::size_t j = 0; j < w; ++j) {
      if (query[i]) {
        out[i*w + j] = content[ix++];
      } else {
        out[i*w + j] = false;
      }
    }
  }
  return out;
};


TEST_CASE("") {
  constexpr std::size_t n = 16;
  constexpr std::size_t w = 12;

  rc::prop("buffer stack correct", [&](std::array<bool, n*w> content, std::array<bool, n> query) {
      std::vector<bool> data = bool_arr_to_vec(content);
      std::vector<bool> q = bool_arr_to_vec(query);

      test_circuit([=]<Mode m> {
        auto the_stack = renc<m>(data);
        const auto qq = renc<m>(q);

        std::vector<bool> out;
        for (const auto& query : qq) {
          const auto popped = epop(w, reveal(query), std::span { the_stack });
          const auto val = dec(std::span { popped });
          std::copy(val.begin(), val.end(), std::back_inserter(out));
        }
        return out;
      }, stack_spec(w, data, q));
    });
}


TEST_CASE("") {
  constexpr std::size_t n = 16;
  constexpr std::size_t w = 3;

  rc::prop("stack correct", [&](std::array<bool, n*w> content, std::array<bool, n> query) {
      std::vector<bool> data = bool_arr_to_vec(content);
      std::vector<bool> q = bool_arr_to_vec(query);

      test_circuit([=]<Mode m> {
        const auto the_stack = renc<m>(data);
        const auto qq = renc<m>(q);

        std::vector<bool> out;
        SimpleStack<m> s(w, the_stack);
        for (const auto& query : qq) {
          const auto popped = s.pop(reveal(query));
          const auto val = dec(std::span { popped });
          std::copy(val.begin(), val.end(), std::back_inserter(out));
        }
        return out;
      }, stack_spec(w, data, q));
    });
}
