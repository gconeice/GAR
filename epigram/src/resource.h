#ifndef RESOURCE_H__
#define RESOURCE_H__


#include <mode.h>
#include <prg.h>
#include <label.h>
#include <test.h>


struct Cost {
  std::size_t rows;
  std::size_t wrows;
  std::size_t bits;
  std::size_t bytes;
  std::size_t nonces;

  constexpr Cost() :
    rows(0), wrows(0), bits(0), bytes(0), nonces(0) { }

  constexpr bool operator==(const Cost& o) const {
    return
      (rows == o.rows) &&
      (wrows == o.wrows) &&
      (bits == o.bits) &&
      (bytes == o.bytes) &&
      (nonces == o.nonces);
  }

  friend std::ostream& operator<<(std::ostream& os, const Cost& c) {
    os << "{\n";
    os << "  rows:   " << c.rows << '\n';
    os << "  wrows:  " << c.wrows << '\n';
    os << "  bits:   " << c.bits << '\n';
    os << "  bytes:  " << c.bytes << '\n';
    os << "  nonces: " << c.nonces << '\n';
    os << "}\n";
    return os;
  }
};


extern Cost gcost;
extern Cost ecost;
extern Cost scost;

extern std::vector<Label> gmat;
extern std::vector<WLabel> gwmat;
extern std::vector<bool> gbits;
extern std::vector<std::byte> gbytes;

extern std::vector<Label>* emat;
extern std::vector<WLabel>* ewmat;
extern std::vector<bool>* ebits;
extern std::vector<std::byte>* ebytes;

template <Mode m>
void initialize(const EPCCRH&);

void ReShowData();
void set_delta(const EPCCRH& key);

Label delta();
WLabel deltas();

Label epdelta();

Label H(const Label&);
Label H(const Label&, std::size_t nonce);

WLabel H(const WLabel&);
WLabel H(const WLabel&, std::size_t nonce);

Label random_label();
WLabel random_wlabel();

void send(const Label&);
Label recv();
void wsend(const WLabel&);
WLabel wrecv();
void sendbit(bool);
bool recvbit();
void sendbyte(std::byte);
std::byte recvbyte();

/**
 * Testing function that executes the GC for G, then for E.
 */
template <typename F>
auto ckt(const F& f) {
  EPCCRH key = EPPRG()();
  initialize<Mode::S>(key);
  f.template operator()<Mode::S>();
  initialize<Mode::G>(key);
  f.template operator()<Mode::G>();
  initialize<Mode::E>(key);
  return f.template operator()<Mode::E>();
}

/**
 * Testing function that garbles function F, then estimates the cost and checks
 * that the estimated cost matches the actual consumed resources.
 */
template <typename F>
bool gate_count_check(const F& f) {
  EPCCRH key = EPPRG()();
  initialize<Mode::S>(key);
  f.template operator()<Mode::S>();
  initialize<Mode::G>(key);
  f.template operator()<Mode::G>();
  initialize<Mode::S>(key);
  f.template operator()<Mode::S>();
  return gcost == scost;
}


template <typename C, typename T>
void test_circuit(const C& c, const T& expected) {
  RC_ASSERT(ckt(c) == expected);
  RC_ASSERT(gate_count_check(c));
}


#endif
