#include <resource.h>

Label _delta;
EPCCRH fixed_key;
EPPRG prg;

std::vector<Label> gmat;
std::vector<WLabel> gwmat;
std::vector<bool> gbits;
std::vector<std::byte> gbytes;

std::vector<Label>* emat;
std::vector<WLabel>* ewmat;
std::vector<bool>* ebits;
std::vector<std::byte>* ebytes;

void ReShowData() {
  std::cout << "HIHIHI " << gmat.size() << ' ' 
                         << gwmat.size() << ' ' 
                         << gbits.size() << ' '
                         << gbytes.size() << std::endl;
  std::cout << "HAHAH "  << gcost.rows << std::endl;
}

Cost gcost;
Cost ecost;
Cost scost;

std::size_t* nonce;

void set_delta(const EPCCRH& key) {
  prg = EPPRG();
  _delta = prg();
  _delta.set_lsb(1);
}

template <>
void initialize<Mode::G>(const EPCCRH& key) {
  fixed_key = key;
  //set_delta(key);
  /*
  prg = EPPRG();
  _delta = prg();
  _delta.set_lsb(1);
  */
  gmat = { };
  gwmat = { };
  gbits = { };
  gbytes = { };

  gmat.reserve(scost.rows);
  gwmat.reserve(scost.wrows);
  gbits.reserve(scost.bits);
  gbytes.reserve(scost.bytes);

  gcost = { };
  nonce = &gcost.nonces;
}


template <>
void initialize<Mode::E>(const EPCCRH& key) {
  fixed_key = key;
  emat = &gmat;
  ewmat = &gwmat;
  ebits = &gbits;
  ebytes = &gbytes;

  ecost = { };
  nonce = &ecost.nonces;
}


template <>
void initialize<Mode::S>(const EPCCRH&) {
  scost = { };
}

Label delta() {
  return _delta;
}


Label epdelta() {
  return _delta;
}

WLabel deltas() {
  return WLabel::broadcast(delta());
}


Label H(const Label& x) {
  return fixed_key(x ^ std::bitset<128> { *nonce });
}

Label H(const Label& x, std::size_t non) {
  return fixed_key(x ^ std::bitset<128> { non });
}

WLabel H(const WLabel& x) {
  const auto nonces = WLabel { std::array<Label, 8> {
    *nonce + 0,
    *nonce + 1,
    *nonce + 2,
    *nonce + 3,
    *nonce + 4,
    *nonce + 5,
    *nonce + 6,
    *nonce + 7
  } };
  return fixed_key(x ^ nonces);
}


WLabel H(const WLabel& x, std::size_t non) {
  const auto nonces = WLabel::broadcast(non);
  return fixed_key(x ^ nonces);
}


void send(const Label& x) {
  gmat[gcost.rows++] = x;
}


Label recv() {
  Label out = (*emat)[ecost.rows++];
  return out;
}


void wsend(const WLabel& x) {
  gwmat[gcost.wrows++] = x;
}


WLabel wrecv() {
  WLabel out = (*ewmat)[ecost.wrows++];
  return out;
}


void sendbit(bool x) {
  gbits[gcost.bits++] = x;
}


bool recvbit() {
  bool out = (*ebits)[ecost.bits];
  ++ecost.bits;
  return out;
}


void sendbyte(std::byte x) {
  gbytes[gcost.bytes++] = x;
}


std::byte recvbyte() {
  std::byte out = (*ebytes)[ecost.bytes];
  ++ecost.bytes;
  return out;
}


Label random_label() { return prg(); }
WLabel random_wlabel() { return prg.wide(); }
