#include <util.h>
#include <permute.h>


template <Mode m>
bool inner_node(SimpleStack<m>& s0, SimpleStack<m>& s1, std::span<Garbled::Bit<m>> input) {
  // At the internal nodes we achieve two core tasks:
  // 1) Setup two stacks that hold the input languages for the child nodes
  // 2) Recursively set up the two child nodes

  // Start by randomly selecting input languages for the children

  // Now, we will immediately simulate n accesses to these two stacks.
  // Note that E actually performs these accesses out-of-band, and so
  // she must separately manipulate resources to make operations work properly.

  // The first bit of each input decides which child is to be accessed
  // Decode dir for E so that she knows which way to navigate
  const auto dir = reveal(input[0]);

  // Pop the stack based on dir to compute the language for the correct child
  const auto pop0 = s0.pop(~dir);
  const auto pop1 = s1.pop(dir);

  // Manipulate the remaining inputs by swapping their language to the one
  // popped from a stack
  input.subspan(1) ^= std::span { pop0 };
  input.subspan(1) ^= std::span { pop1 };
  open(input.subspan(1));

  return color(dir);
}


template <template <Mode> typename Leaf, Mode m>
void setup_lazy_permutation(
    const Leaf<m>& leaf,
    std::size_t start,
    std::size_t stop,
    std::span<SimpleStack<m>> stacks,
    std::span<Garbled::Bit<m>> input_languages,
    std::span<Cost> resource_positions) {
  // Note that E does not set up the nodes!
  // This is because her execution is decided at access time, not when the
  // lazy permutation is constructed.
  // However, she does *speculatively* set up to determine the resource
  // position for each internal node.

  const auto n = stop - start;
  const auto logn = log2(n);
  const auto inp = leaf.ninp();
  const auto out = leaf.nout();
  const auto w = logn + inp + out;

  if constexpr (m == Mode::S) {
    // save the starting location of the GC
    resource_positions[0] = scost;
  }

  if (n > 1) {
    // Populate stacks with random languages
    // Note that this does not imply communication
    auto ls = random_garbled_string<m>(n*(w-1));
    auto lspan = std::span { ls };
    auto ls0 = lspan.subspan(0, (n/2) * (w-1));
    auto ls1 = lspan.subspan((n/2) * (w-1));

    stacks[0] = SimpleStack<m>(w-1, ls0);
    stacks[1] = SimpleStack<m>(w-1, ls1);

    if (m != Mode::E) {
      // G sets up all accesses now, E defers them
      for (std::size_t i = 0; i < n; ++i) {
        inner_node(stacks[0], stacks[1], input_languages.subspan(w*i, w));
      }
    }

    // Recursively set up children
    setup_lazy_permutation(
        leaf,
        start,
        start + n/2,
        stacks.subspan(2, n - 2),
        std::span { ls0 },
        resource_positions.subspan(1, n-1));
    setup_lazy_permutation(
        leaf,
        stop - n/2,
        stop,
        stacks.subspan(n, n - 2),
        std::span { ls1 },
        resource_positions.subspan(n, n-1));
  } else {
    if constexpr (m != Mode::E) {
      // E delays actual accesses to the nodes

      const auto leaf_inp = input_languages.subspan(0, leaf.ninp());
      const auto leaf_out = input_languages.subspan(leaf.ninp());
      auto res = leaf(start, leaf_inp);

      assert(leaf_out.size() == leaf.nout());
      assert(res.size() == leaf.nout());

      std::span { res } ^= leaf_out;
      open(std::span { res });
    }
  }
}


template <template <Mode> typename Leaf, Mode m>
LazyPermutation<Leaf, m>::LazyPermutation(std::size_t n, const Leaf<m>& l)
  : leaf(l), n(n), timer(0) {

  assert(is_pow2(n));
  const auto logn = log2(n);
  stacks.resize(2*n-2);

  const auto inp = leaf.ninp();
  const auto out = leaf.nout();

  // For each access, our input language requires
  // 1) `w` labels to support the output
  // 2) `log(n)` labels to navigate recursively to the correct leaf
  input_languages = random_garbled_string<m>(n * (logn + inp + out));

  resource_positions.resize(2*n-1);

  auto lang_copy = input_languages;
  setup_lazy_permutation(
      leaf,
      0, n,
      std::span { stacks },
      std::span<Garbled::Bit<m>> { lang_copy },
      std::span { resource_positions });

  if constexpr (m == Mode::E) {
    // E speculatively determines all stack costs
    std::vector<Garbled::Bit<Mode::S>> slang(input_languages.size());
    std::vector<SimpleStack<Mode::S>> sstacks(stacks.size());

    scost = ecost;

    setup_lazy_permutation(
        leaf.speculate(),
        0, n,
        std::span { sstacks },
        std::span { slang },
        std::span { resource_positions });

    ecost = scost;
  }
}


template LazyPermutation<LookupLeaf, Mode::G>::LazyPermutation(std::size_t, const LookupLeaf<Mode::G>&);
template LazyPermutation<LookupLeaf, Mode::E>::LazyPermutation(std::size_t, const LookupLeaf<Mode::E>&);
template LazyPermutation<LookupLeaf, Mode::S>::LazyPermutation(std::size_t, const LookupLeaf<Mode::S>&);


template <template <Mode> typename Leaf, Mode m>
void access_tree(
    const Leaf<m>& leaf,
    std::size_t start,
    std::size_t stop,
    std::span<SimpleStack<m>> stacks,
    std::span<Garbled::Bit<m>> input,
    std::span<Cost> resource_positions,
    std::span<Garbled::Bit<m>> out) {

  const auto n = stop - start;

  // Note that G does not perform accesses!
  // He has already prepared for all accesses at initialization.

  if (n > 1) {
    // Lookup the next GC for this node
    ecost = resource_positions[0];
    // Use the node

    bool dir = inner_node(stacks[0], stacks[1], input);

    // Save the location of the next GC for this node
    resource_positions[0] = ecost;

    if (!dir) {
      access_tree(
          leaf,
          start,
          start + n/2,
          stacks.subspan(2, n-2),
          input.subspan(1),
          resource_positions.subspan(1, n-1),
          out);
    } else {
      access_tree(
          leaf,
          stop - n/2,
          stop,
          stacks.subspan(n, n-2),
          input.subspan(1),
          resource_positions.subspan(n, n-1),
          out);
    }
  } else {
    // At the leaf, we simply invoke the leaf circuit and translate the result
    ecost = resource_positions[0];

    const auto leaf_inp = input.subspan(0, leaf.ninp());
    const auto leaf_out = input.subspan(leaf.ninp());
    const auto res = leaf(start, leaf_inp);

    assert(leaf_out.size() == leaf.nout());
    assert(res.size() == leaf.nout());

    out ^= std::span { res };
    out ^= std::span { leaf_out };
    open(out);

    resource_positions[0] = ecost;
  }
}



template <template <Mode> typename Leaf, Mode m>
std::vector<Garbled::Bit<m>> LazyPermutation<Leaf, m>::operator()(
    std::span<const Garbled::Bit<m>> ix,
    std::span<const Garbled::Bit<m>> circuit_input) {
  const auto logn = log2(n);
  const auto ninp = leaf.ninp();
  const auto nout = leaf.nout();

  assert(timer < n);
  assert(ix.size() == logn);

  const auto w = logn + ninp + nout;

  auto languages = std::span { input_languages };

  auto out = random_garbled_string<m>(nout);

  std::vector<Garbled::Bit<m>> input(w);
  auto inp = std::span { input };
  std::copy(ix.rbegin(), ix.rend(), inp.begin());
  inp.subspan(logn, ninp) ^= circuit_input;
  inp.subspan(logn + ninp) ^= std::span { out };
  inp ^= languages.subspan(w*timer, w);
  open(inp);

  if constexpr (m == Mode::E) {
    const auto current_position = ecost;
    access_tree(
        leaf,
        0,
        n,
        std::span { stacks },
        std::span { input },
        std::span { resource_positions },
        std::span { out });
    ecost = current_position;
  }
  ++timer;

  return out;
}

template std::vector<Garbled::Bit<Mode::G>> LazyPermutation<LookupLeaf, Mode::G>::operator()(
    std::span<const Garbled::Bit<Mode::G>>, std::span<const Garbled::Bit<Mode::G>>);
template std::vector<Garbled::Bit<Mode::E>> LazyPermutation<LookupLeaf, Mode::E>::operator()(
    std::span<const Garbled::Bit<Mode::E>>, std::span<const Garbled::Bit<Mode::E>>);
template std::vector<Garbled::Bit<Mode::S>> LazyPermutation<LookupLeaf, Mode::S>::operator()(
    std::span<const Garbled::Bit<Mode::S>>, std::span<const Garbled::Bit<Mode::S>>);


TEST_CASE("") {
  constexpr std::size_t size = 8;
  rc::prop("Lazy permutation setup cost estimation correct",
    [&](bool, std::array<bool, size> content) {
      std::vector<bool> c(size);
      std::copy(content.begin(), content.end(), c.begin());


      RC_ASSERT(gate_count_check([&]<Mode m> {
        LookupLeaf<m> leaf(1, renc<m>(c));
        LazyPermutation<LookupLeaf, m>(size, leaf);
      }));
    });
}


TEST_CASE("") {
  constexpr std::size_t size = 32;
  constexpr std::size_t num_accesses = 32;
  rc::prop("Lazy permutation correct",
    [&](std::array<bool, size> content) {
      const auto o = random_permutation(EPPRG()(), size);
      std::vector<std::vector<bool>> order;
      for (auto i : o) {
        std::vector<bool> index;
        for (std::size_t j = 0; j < log2(size); ++j) {
          index.push_back(i & 1);
          i >>= 1;
        }
        order.push_back(index);
      }


      std::vector<bool> c(size);
      std::copy(content.begin(), content.end(), c.begin());

      const auto actual = ckt([&]<Mode m> {
        LookupLeaf leaf(1, renc<m>(c));
        auto a = LazyPermutation<LookupLeaf, m>(size, leaf);

        std::vector<bool> out(num_accesses);
        for (std::size_t i = 0; i < num_accesses; ++i) {
          const auto ord = renc<m>(order[i]);
          std::span<Garbled::Bit<m>> inp_span;
          out[i] = dec(a(std::span { ord }, inp_span)[0]);
        }
        return out;
      });

      std::vector<bool> expected(num_accesses);
      for (std::size_t i = 0; i < num_accesses; ++i) {
        expected[i] = content[o[i]];
      }

      RC_ASSERT(actual == expected);
    });
}
