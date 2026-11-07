#include <am.h>
#include <klib.h>
#define ITERS 1000
static int8_t va[8] __attribute__((aligned(16))) = {1,2,3,4,1,2,3,4};
static int8_t vb[8] __attribute__((aligned(16))) = {3,2,1,3,2,1,3,2};
int main(void) {
  printf("=== scalar 8-MAC baseline, iters=%d ===\n", ITERS);
  long s = 0;
  for (int it = 0; it < ITERS; it++) {
    int acc = 0;
    for (int k = 0; k < 8; k++) acc += (int)va[k] * (int)vb[k];
    s = acc;
  }
  printf("sum = %ld  expected 43  %s\n", s, s == 43 ? "PASS" : "FAIL");
  printf("SCALAR DONE\n");
  return 0;
}
