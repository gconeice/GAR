#ifndef MODE_H__
#define MODE_H__


#include <iostream>
#include <tuple>


enum class Mode {
  G, // GC Generator
  E, // GC Evaluator
  S, // Cost speculation
};


#define GEN if constexpr (m == Mode::G)
#define EVAL if constexpr (m == Mode::E)
#define SPEC if constexpr (m == Mode::S)


struct Dummy {
  friend std::ostream& operator<<(std::ostream& os, Dummy) {
    os << "·";
    return os;
  }
};

inline Dummy operator&(Dummy, Dummy) { return Dummy { }; } 

extern Dummy the_dummy;

template <typename T, Mode m>
struct Tuple;


namespace std {
template<typename T, Mode m>
struct tuple_size<::Tuple<T, m>> {
  static constexpr size_t value = 2;
};
}


template <typename T>
struct Tuple<T, Mode::G> {
public:
  constexpr Tuple() { }
  constexpr Tuple(const T& g) : g(g) { }
  constexpr Tuple(const T& g, Dummy) : g(g) { }

  T& G() { return g; }
  Dummy& E() { return the_dummy; }

  const T& G() const { return g; }
  const Dummy& E() const { return the_dummy; }

  template<std::size_t Index>
  std::tuple_element_t<Index, Tuple<T, Mode::G>>& get() {
    if constexpr (Index == 0) return g;
    if constexpr (Index == 1) return the_dummy;
  }

  template<std::size_t Index>
  const std::tuple_element_t<Index, Tuple<T, Mode::G>>& get() const {
    if constexpr (Index == 0) return g;
    if constexpr (Index == 1) return the_dummy;
  }

private:
  T g;
};


template <typename T>
struct Tuple<T, Mode::E> {
public:
  constexpr Tuple() { }
  constexpr Tuple(const T& e) : e(e) { }
  constexpr Tuple(Dummy, const T& e) : e(e) { }

  Dummy& G() { return the_dummy; }
  T& E() { return e; }

  const Dummy& G() const { return the_dummy; }
  const T& E() const { return e; }

  template<std::size_t Index>
  std::tuple_element_t<Index, Tuple<T, Mode::E>>& get() {
    if constexpr (Index == 0) return the_dummy;
    if constexpr (Index == 1) return e;
  }

  template<std::size_t Index>
  const std::tuple_element_t<Index, Tuple<T, Mode::E>>& get() const {
    if constexpr (Index == 0) return the_dummy;
    if constexpr (Index == 1) return e;
  }

private:
  T e;
};


template <typename T>
struct Tuple<T, Mode::S> {
public:
  constexpr Tuple() { }
  constexpr Tuple(Dummy, Dummy) { }

  Dummy& G() { return the_dummy; }
  Dummy& E() { return the_dummy; }
  const Dummy& G() const { return the_dummy; }
  const Dummy& E() const { return the_dummy; }

  template<std::size_t Index>
  std::tuple_element_t<Index, Tuple<T, Mode::S>>& get() {
    if constexpr (Index == 0) return the_dummy;
    if constexpr (Index == 1) return the_dummy;
  }

  template<std::size_t Index>
  const std::tuple_element_t<Index, Tuple<T, Mode::S>>& get() const {
    if constexpr (Index == 0) return the_dummy;
    if constexpr (Index == 1) return the_dummy;
  }
};


template <typename T, Mode m>
std::ostream& operator<<(std::ostream& os, const Tuple<T, m>& t) {
  os << "〈 " << t.G() << " , " << t.E() << " 〉";
  return os;
}


namespace std {
template<typename T, size_t Index>
struct tuple_element<Index, ::Tuple<T, Mode::G>>
  : conditional<Index == 0, T, Dummy> {
  static_assert(Index < 2, "Index out of bounds for Tuple");
};

template<typename T, size_t Index>
struct tuple_element<Index, ::Tuple<T, Mode::E>>
  : conditional<Index == 0, Dummy, T> {
  static_assert(Index < 2, "Index out of bounds for Tuple");
};

template<typename T, size_t Index>
struct tuple_element<Index, ::Tuple<T, Mode::S>>
  : conditional<Index == 0, Dummy, Dummy> {
  static_assert(Index < 2, "Index out of bounds for Tuple");
};
}


#endif
