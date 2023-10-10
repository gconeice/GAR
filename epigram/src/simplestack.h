#ifndef SIMPLE_STACK_H__
#define SIMPLE_STACK_H__


#include <linearscan.h>


template <Mode m>
struct SimpleStack {
public:
  SimpleStack() { }
  SimpleStack(std::size_t w, std::span<const Garbled::Bit<m>>);
  std::vector<Garbled::Bit<m>> pop(const Garbled::Bit<m>&);

private:
  std::size_t w;
  std::size_t timer;
  std::vector<Garbled::Bit<m>> content;
  std::vector<Garbled::Bit<m>> vacancies;
};


#endif
