#include <stack.h>
#include <util.h>


template <Mode m>
Stack<m>::Stack(std::size_t w, const std::vector<Garbled::Bit<m>>& c)
  : w(w)
  , timer(0)
  , content(c) {

  assert(content.size() % w == 0);

  const auto n = content.size() / w;

  const auto levels = log2((n+2)/3 +1);
  const auto full_size = w*((1 << levels) - 1)*3;
  content.resize(full_size);

  metadata.resize(levels);
}

template Stack<Mode::G>::Stack(std::size_t, const std::vector<Garbled::Bit<Mode::G>>&);
template Stack<Mode::E>::Stack(std::size_t, const std::vector<Garbled::Bit<Mode::E>>&);
template Stack<Mode::S>::Stack(std::size_t, const std::vector<Garbled::Bit<Mode::S>>&);


/**
 * The recursive strategy for popping a stack.
 * This procedure allows the user (namely, itself) to pop the top of the stack
 * into one of two slots (or neither).
 *
 * `target` should be a two bit flag indicating how far to "shift out" the read value.
 * `target` can take on three values:
 * 00 -> no read
 * 10 -> (i.e. target[0] = 1, target[1] = 0) pop a value into output slot 0
 * 01 -> pop a value into output slot 1
 * 11 -> illegal
 *
 * Thus `output` should have length `2*w`.
 */
template <Mode m>
void pop_level(
    std::size_t w,
    std::size_t timer,
    std::span<Garbled::Bit<m>> content,
    std::span<typename Stack<m>::Metadata> metadata,
    std::span<const Garbled::Bit<m>> target, // two bits indicating how far to shift out the popped value
    std::span<Garbled::Bit<m>> out) {

  assert(content.size() >= 3*w);
  assert(out.size() == 2*w);
  assert(metadata.size() >= 1);

  std::span<const Garbled::Bit<m>> c = content;
  auto& tags = metadata[0];

  // If either target is marked as one, we are performing a read
  const auto should_we_read = target[0] ^ target[1];

  // add a vacancy, if we are reading
  // note, we add this before handling the actual popping
  tags.vacancy_counter[1] ^= eand(tags.vacancy_counter[0], should_we_read);
  tags.vacancy_counter[0] ^= should_we_read;


  if ((timer & 1) == 0) {
    // On even cycles, we simply read slot 0.
    const auto read = onehot_op(target, c.subspan(0, w));

    for (std::size_t i = 0; i < w; ++i) {
      // If we indeed read slot zero, we should clear its contents
      content[i] ^= read[w + i] ^ read[2*w + i];

      // Move the correct rows of the one hot product into the output
      out[i] = read[w + i];
      out[w+i] = read[2*w + i];
    }
    // If we read, mark slot 0 as vacant.
    tags.slot_zero_vacant = should_we_read;

    // read contains four values, but we are only interested in the first three
  } else {
    // On odd cycles we
    // (1) conditionally read slot 0
    // (2) conditionally read slot 1 / shift slot 1 into slot 0
    // (3) conditionally read slot 2 / shift slot 2 into either slot 0 or 1 as appropriate
    // (4) recursively pop the next level into the first two available vacant slots (if possible)

    // one hot encoding of the number of vacancies
    std::array<Garbled::Bit<m>, 4> vac_map;
    vac_map[3] = eand(tags.vacancy_counter[0], tags.vacancy_counter[1]);
    vac_map[2] = vac_map[3] ^ tags.vacancy_counter[1];
    vac_map[1] = vac_map[3] ^ tags.vacancy_counter[0];
    vac_map[0] = constant<m>(1) ^ vac_map[1] ^ vac_map[2] ^ vac_map[3];

    { // (1) conditionally read slot 0
      // If slot 0 is vacated, a read is harmless, so we can use the target as is.
      const auto read0 = onehot_op(target, c.subspan(0, w));
      for (std::size_t i = 0; i < w; ++i) {
        content[i] ^= read0[w + i] ^ read0[2*w + i];
        out[i] ^= read0[w + i];
        out[w+i] ^= read0[2*w + i];
      }
    }

    { // (2) conditionally read slot 1 / shift slot 1 into slot 0
      // To read from level slot 1 we use the following map:
      // 00 -> no read
      // 10 -> read into output slot 0
      // 01 -> read into output slot 1
      // 11 -> shift left into level slot 0
      std::array<Garbled::Bit<m>, 2> target1;
      const auto reading_slot_0 = eand(should_we_read, ~tags.slot_zero_vacant);
      const auto reading_slot_1 = eand(should_we_read, tags.slot_zero_vacant);
      target1[0] = eand(reading_slot_1, target[1]) ^ reading_slot_0 ^ tags.slot_zero_vacant;
      target1[1] = eand(reading_slot_1, target[0]) ^ reading_slot_0 ^ tags.slot_zero_vacant;


      const auto read1 = onehot_op(std::span<const Garbled::Bit<m>> { target1 }, c.subspan(w, w));
      for (std::size_t i = 0; i < w; ++i) {
        content[w + i] ^= read1[w + i] ^ read1[2*w + i] ^ read1[3*w + i];
        out[i] ^= read1[w + i];
        out[w + i] ^= read1[2*w + i];
        content[i] ^= read1[3*w + i];
      }
    }

    { // (3) conditionally read slot 2 / shift slot 2 into either slot 0 or 1 as appropriate
      // To read from level slot 2 we use the following map:
      // 000 -> no read
      // 100 -> read into output slot 0
      // 010 -> read into output slot 1
      // 101 -> shift left into level slot 0
      // 011 -> shift left into level slot 1

      std::array<Garbled::Bit<m>, 3> target2;

      // Are we popping the third slot into one of the output slots?
      const auto pop_to_0 = eand(target[0], vac_map[3]);
      const auto pop_to_1 = eand(target[1], vac_map[3]);

      // Bit 0 is 1 iff:
      // - We are popping the slot to output 0
      // - There are two vacancies
      target2[0] = pop_to_0 ^ vac_map[2];

      // Bit 1 is 1 iff:
      // - We are popping the slot to output 1
      // - There is one vacancy
      target2[1] = pop_to_1 ^ vac_map[1];

      // Bit 2 is 1 iff:
      // - There are at least one vacancy, but we are not popping from this slot
      target2[2] = eand(~(pop_to_0 ^ pop_to_1), vac_map[1] ^ vac_map[2] ^ vac_map[3]);

      const auto read2 = onehot_op(std::span<const Garbled::Bit<m>> { target2 }, c.subspan(2*w, w));
      for (std::size_t i = 0; i < w; ++i) {
        content[2*w + i] ^= read2[w + i] ^ read2[2*w + i] ^ read2[5*w + i] ^ read2[6*w + i];
        out[i] ^= read2[w + i];
        out[w + i] ^= read2[2*w + i];
        content[i] ^= read2[5*w + i];
        content[w + i] ^= read2[6*w + i];
      }
    }


    // (4) recursively pop the next level into the first two available vacant slots (if possible)
    if (metadata.size() > 1) { // only recurse if next level exists
      std::array<Garbled::Bit<m>, 2> rec_target;
      // pop to left target if three vacancies
      rec_target[0] = vac_map[3];
      // pop to right target if only two vacancies
      rec_target[1] = vac_map[2];

      std::vector<Garbled::Bit<m>> rec_out(4*w);
      pop_level(
          w << 1,
          timer >> 1,
          content.subspan(3*w),
          metadata.subspan(1),
          std::span<const Garbled::Bit<m>> { rec_target },
          std::span { rec_out });

      // Now load the recursive output into the slots
      for (std::size_t i = 0; i < w; ++i) {
        content[i] ^= rec_out[i];
        content[w + i] ^= rec_out[w + i] ^ rec_out[2*w + i];
        content[2*w + i] ^= rec_out[3*w + i];
      }

      // we now definitely have fewer than two vacancies 
      tags.vacancy_counter[1] = constant<m>(false);
    }

    // slot zero is definitely filled
    tags.slot_zero_vacant = constant<m>(false);
  }
}

template <Mode m>
void Stack<m>::pop(const Garbled::Bit<m>& p, std::span<Garbled::Bit<m>> out) {
  std::array<Garbled::Bit<m>, 2> target = { p, constant<m>(0) };

  // we need a double wide buffer because pop level tries to write to two locations
  std::vector<Garbled::Bit<m>> buffer(2*w);
  pop_level(
      w,
      timer,
      std::span { content },
      std::span { metadata },
      std::span<const Garbled::Bit<m>> { target },
      std::span { buffer });
  std::copy(buffer.begin(), buffer.begin() + w, out.begin());
  ++timer;
}


template void Stack<Mode::G>::pop(const Garbled::Bit<Mode::G>&, std::span<Garbled::Bit<Mode::G>>);
template void Stack<Mode::E>::pop(const Garbled::Bit<Mode::E>&, std::span<Garbled::Bit<Mode::E>>);
template void Stack<Mode::S>::pop(const Garbled::Bit<Mode::S>&, std::span<Garbled::Bit<Mode::S>>);


/* TEST_CASE("") { */
/*   struct Spec { */
/*     Spec() { } */
/*     Spec(const std::vector<std::size_t>& c) : content(c), ix(0) { } */

/*     std::size_t pop(bool q) { */
/*       if (q) { */
/*         auto out = content[ix]; */
/*         ++ix; */
/*         return out; */
/*       } else { */
/*         return false; */
/*       } */
/*     } */

/*     std::vector<std::size_t> content; */
/*     std::size_t ix; */
/*   }; */

/*  const auto pattern = [](const std::vector<std::size_t>& content, const std::vector<bool>& query) { */
/*     Spec s(content); */
/*     std::vector<std::size_t> out(content.size()); */
/*     for (std::size_t i = 0; i < content.size(); ++i) { */
/*       out[i] = s.pop(query[i]); */
/*     } */
/*     return out; */
/*   }; */

/*   constexpr std::size_t size = 4; */
/*   constexpr std::size_t w = 3; */
/*   rc::prop("stack correct", */
/*     [&](std::array<std::size_t, size> content, std::array<bool, size> query) { */
/*       std::vector<std::size_t> data(size); */
/*       std::vector<bool> q(size); */

/*       for (std::size_t i = 0; i < size; ++i) { */
/*         data[i] = content[i] % (1 << w); */
/*       } */

/*       std::copy(query.begin(), query.end(), q.begin()); */

/*       const auto c = [=]<Mode m> { */
/*         std::vector<Garbled::Bit<m>> cc(w*size); */
/*         for (std::size_t i = 0; i < size; ++i) { */
/*           const auto v = enc_int<m>(w, data[i]); */
/*           for (std::size_t j = 0; j < w; ++j) { */
/*             cc[i*w + j] = v[j]; */
/*           } */
/*         } */
/*         const auto qq = enc<m>(q); */

/*         std::vector<Garbled::Bit<m>> buffer(w); */
/*         std::vector<std::size_t> out(qq.size()); */

/*         Stack<m> s(w, cc); */
/*         for (std::size_t i = 0; i < qq.size(); ++i) { */
/*           s.pop(qq[i], std::span { buffer }); */
/*           out[i] = dec_int(buffer); */
/*         } */
/*         return out; */
/*       }; */
/*       RC_ASSERT(pattern(data, q) == ckt(c)); */
/*       RC_ASSERT(gate_count_check(c)); */
/*     }); */
/* } */


/* TEST_CASE("") { */
/*   constexpr std::size_t size = 6; */
/*   constexpr std::size_t w = 1; */
/*   rc::prop("overpopping stack", */
/*     [&](std::array<std::size_t, size> content) { */
/*       std::vector<std::size_t> data(size); */
/*       std::vector<bool> query(size*2); */
/*       for (std::size_t i = 0; i < size; ++i) { query[i] = true; } */
/*       for (std::size_t i = 0; i < size; ++i) { query[i+size] = false; } */

/*       for (std::size_t i = 0; i < size; ++i) { */
/*         data[i] = content[i] % (1 << w); */
/*       } */

/*       const auto actual = ckt([=]<Mode m> { */
/*         std::vector<Garbled::Bit<m>> cc(w*size); */
/*         for (std::size_t i = 0; i < size; ++i) { */
/*           const auto v = enc_int<m>(w, data[i]); */
/*           for (std::size_t j = 0; j < w; ++j) { */
/*             cc[i*w + j] = v[j]; */
/*           } */
/*         } */
/*         const auto qq = enc<m>(query); */

/*         std::vector<Garbled::Bit<m>> buffer(w); */
/*         std::vector<std::size_t> out(qq.size()); */

/*         Stack<m> s(w, cc); */
/*         for (std::size_t i = 0; i < qq.size(); ++i) { */
/*           s.pop(qq[i], std::span { buffer }); */
/*           out[i] = dec_int(buffer); */
/*         } */
/*         return out; */
/*       }); */
/*       for (std::size_t i = 0; i < size; ++i) { */
/*         RC_ASSERT(actual[i+size] == 0); */
/*       } */
/*     }); */
/* } */

