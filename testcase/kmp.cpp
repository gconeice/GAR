#define MAX 100
int kmp(int *len, char *pattern, char *text) {
  int plength = len[0];
  int tlength = len[1];
  int lps[MAX];
  for (int i = 1; i < plength; i++) {
    int j;
    for (j = lps[i-1]; j && pattern[j] != pattern[i]; j = lps[j-1]);
    lps[i] = pattern[j] == pattern[i] ? j + 1 : j;
  }
  int res = 0;
  int now = 0;
  for (int i = 0; i < tlength; i++) {
    for (; now && pattern[now] != text[i]; now = lps[now-1]);
    now = pattern[now] == text[i] ? now + 1 : now;
    if (now == plength) {
      res++;
      now = lps[now-1];
    }
  }
  return res;
}
