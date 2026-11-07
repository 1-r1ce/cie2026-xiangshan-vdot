#include <am.h>
#include <klib.h>
#define ITERS 1000
static int8_t va[8] __attribute__((aligned(16))) = {1,2,3,4,1,2,3,4};
static int8_t vb[8] __attribute__((aligned(16))) = {3,2,1,3,2,1,3,2};
int main(void) {
  printf("=== stdvec(vmul.vv+vredsum.vs) bench, iters=%d ===\n", ITERS);
  __asm__ volatile("csrs mstatus, %0" :: "r"((uint64_t)0x600) : "memory");
  long out = 0;
  __asm__ volatile("vsetvli zero, %0, e8, m1, ta, ma" :: "r"((uint64_t)8) : "memory");
  __asm__ volatile("vle8.v v1, (%0)" :: "r"(va) : "memory");
  __asm__ volatile("vle8.v v2, (%0)" :: "r"(vb) : "memory");
  for (int it = 0; it < ITERS; it++) {
    __asm__ volatile("vmul.vv v3, v1, v2" ::: "memory");
    __asm__ volatile("vredsum.vs v4, v3, v0" ::: "memory");
  }
  __asm__ volatile("vmv.x.s %0, v4" : "=r"(out));
  printf("lane0 = %ld  (本配置 vredsum 归约结果与规范不符，此程序仅用于计时)\n", (long)(signed char)out);
  printf("STDVEC DONE\n");
  return 0;
}
