#include <am.h>
#include <klib.h>
#define ITERS 1000
static int8_t va[8] __attribute__((aligned(16))) = {1,2,3,4,1,2,3,4};
static int8_t vb[8] __attribute__((aligned(16))) = {3,2,1,3,2,1,3,2};
int main(void) {
  printf("=== vdot.vv latency bench, iters=%d, vl=8 SEW=8 ===\n", ITERS);
  __asm__ volatile("csrs mstatus, %0" :: "r"((uint64_t)0x600) : "memory");
  long out = 0;
  __asm__ volatile("vsetvli zero, %0, e8, m1, ta, ma" :: "r"((uint64_t)8) : "memory");
  __asm__ volatile("vle8.v v1, (%0)" :: "r"(va) : "memory");
  __asm__ volatile("vle8.v v2, (%0)" :: "r"(vb) : "memory");
  for (int it = 0; it < ITERS; it++) {
    __asm__ volatile(".word 0x0020818B" ::: "memory");   /* vdot.vv v3, v1, v2 */
  }
  __asm__ volatile("vmv.x.s %0, v3" : "=r"(out));
  printf("sum = %ld  expected 43  %s\n", out, out == 43 ? "PASS" : "FAIL");
  printf("VDOT DONE\n");
  return 0;
}
