#ifndef STACK_H__
#define STACK_H__


#include <onehot.h>


template <Mode m>
struct Stack {
public:
  struct Metadata {
    std::array<Garbled::Bit<m>, 2> vacancy_counter;
    Garbled::Bit<m> slot_zero_vacant;
  };

  Stack() { }
  Stack(std::size_t w, const std::vector<Garbled::Bit<m>>& content);
  void pop(const Garbled::Bit<m>&, std::span<Garbled::Bit<m>>);

private:
  std::size_t w;
  std::size_t timer;
  std::vector<Garbled::Bit<m>> content;
  std::vector<Metadata> metadata;
};


#endif
