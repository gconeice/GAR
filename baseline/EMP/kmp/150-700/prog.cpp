#include "emp-sh2pc/emp-sh2pc.h"
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
using namespace emp;
using namespace std;


Integer linearscan(Integer *a, Integer idx, int len) {
  Integer res = Integer(32, 0, PUBLIC);
  for (int i = 0; i < len; i++) {
    Bit mask = idx == Integer(32, i, PUBLIC);
    for (int b = 0; b < 32; b++) res[b] = res[b] ^ (mask & a[i][b]);
  }
  return res;
}

Integer condinc(Bit &cond) {
  Integer one = Integer(32, 1, PUBLIC);
  one[0] = one[0] & cond;
  return one;
}

#define PATLEN 150
#define TXTLEN 700

void test_kmp(int party) {
  
  Integer *pat = new Integer[PATLEN];
  Integer *txt = new Integer[TXTLEN];
  for (int i = 0; i < PATLEN; i++) pat[i] = Integer(32, 0, ALICE);
  for (int i = 0; i < TXTLEN; i++) txt[i] = Integer(32, 0, BOB);

  Integer *lps = new Integer[PATLEN];
  for (int i = 0; i < PATLEN; i++) lps[i] = Integer(32, 0, PUBLIC);

  for (int i = 1; i < PATLEN; i++) {
    Integer j = lps[i-1];
    for (int k = 0; k < i; k++) {
      Integer pre = linearscan(lps, j-Integer(32, 1, PUBLIC), PATLEN);
      Bit mask = (j != Integer(32, 0, PUBLIC)) & (pat[i] != linearscan(pat, j, PATLEN));
      for (int b = 0; b < 32; b++) j[b] = (j[b] & !mask) ^ (pre[b] & mask);
    }
    Bit cond = pat[i] == linearscan(pat, j, PATLEN);
    lps[i] = j + condinc(cond);
  }

  Integer res = Integer(32, 0, PUBLIC);
  Integer now = Integer(32, 0, PUBLIC);

  for (int i = 0; i < TXTLEN; i++) {
    for (int k = 0; k < PATLEN; k++) {
      Integer pre = linearscan(lps, now-Integer(32, 1, PUBLIC), PATLEN);
      Bit mask = (now != Integer(32, 0, PUBLIC)) & (txt[i] != linearscan(pat, now, PATLEN));
      for (int b = 0; b < 32; b++) now[b] = (now[b] & !mask) ^ (pre[b] & mask);
    }
    Bit cond = txt[i] == linearscan(pat, now, PATLEN);
    now = now + condinc(cond);
    cond = now == Integer(32, PATLEN, PUBLIC);
    res = res + condinc(cond);
    Integer pre = linearscan(lps, now-Integer(32, 1, PUBLIC), PATLEN);
    for (int b = 0; b < 32; b++) now[b] = (now[b] & !cond) ^ (pre[b] & cond);
  }
  
  cout << res.reveal<uint32_t>() << endl;
  
}


int main(int argc, char** argv) {
  int port, party;
  parse_party_and_port(argv, &party, &port);
  int num = 20;
  if(argc > 3)
    num = atoi(argv[3]);
  NetIO * io = new NetIO(party==ALICE ? nullptr : "127.0.0.1", port);

  setup_semi_honest(io, party);
  test_kmp(party);
  cout << CircuitExecution::circ_exec->num_and()<<endl;
  finalize_semi_honest();
  delete io;
}
