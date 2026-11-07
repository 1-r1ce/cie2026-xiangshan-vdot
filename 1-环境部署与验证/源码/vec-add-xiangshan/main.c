/*
 * 向量加法指令 vadd.vv 执行过程演示（阶段一交付物 S1-3 的程序本体）
 * 平台：XiangShan MinimalConfig（昆明湖 V2 微架构，VLEN=128）+ Verilator 5.052
 * 部署位置：nexus-am/apps/vec-add-xiangshan/main.c
 * 编译：make ARCH=riscv64-xs MARCH=rv64gcv CROSS_COMPILE=riscv64-linux-gnu-
 * 运行：./build/emu --no-diff -i <bin> -C 400000 --force-dump-result
 */
#include <am.h>
#include <klib.h>
#include <stdint.h>

#define N 8
static int32_t __attribute__((aligned(16))) va[N]  = {1,2,3,4,5,6,7,8};
static int32_t __attribute__((aligned(16))) vb[N]  = {10,20,30,40,50,60,70,80};
static int32_t __attribute__((aligned(16))) vc[N];
static int32_t vref[N];

static inline uint64_t rdc(void){ uint64_t x; asm volatile("rdcycle %0":"=r"(x)); return x; }
static inline uint64_t rdi(void){ uint64_t x; asm volatile("rdinstret %0":"=r"(x)); return x; }

static void print_vec(const char *tag, int32_t *v, int n) {
  int i;
  printf("%s = {", tag);
  for (i = 0; i < n; i++) printf("%d%s", (int)v[i], i == n-1 ? "" : ",");
  printf("}\n");
}

int main() {
  uint64_t mst, vl1 = 0, vl2 = 0, c0, c1, i0, i1;
  int i, ok;

  printf("=== XiangShan 向量加法指令 vadd.vv 执行过程 ===\n");
  printf("Team: panpan最努力 | 齐鲁师范学院\n\n");

  /* [0] mstatus.VS 必须为 Initial/Clean/Dirty，否则向量指令非法（裸机环境默认是 Off，
   *     直接跑向量指令会静默停摆：instrCnt 冻结、无异常输出） */
  asm volatile("csrr %0, mstatus" : "=r"(mst));
  printf("[0] mstatus = 0x%x (VS field = %d)\n", (unsigned)mst, (int)((mst >> 9) & 3));
  asm volatile("csrs mstatus, %0" :: "r"((uint64_t)0x600) : "memory");   /* VS = 01b */
  asm volatile("csrr %0, mstatus" : "=r"(mst));
  printf("    置 VS=Initial 后 mstatus = 0x%x (VS field = %d)\n\n",
         (unsigned)mst, (int)((mst >> 9) & 3));

  /* [1] vsetvli 探测 VLEN：e32,m1 -> vl = VLEN/32 */
  asm volatile("vsetvli %0, zero, e32, m1, ta, ma" : "=r"(vl1) :: "memory");
  asm volatile("vsetvli %0, zero, e32, m2, ta, ma" : "=r"(vl2) :: "memory");
  printf("[1] vsetvli zero, e32, m1  -> vl=%d  => VLEN = %d bit\n", (int)vl1, (int)vl1*32);
  printf("    vsetvli zero, e32, m2  -> vl=%d  (VLMAX 随 LMUL 翻倍)\n\n", (int)vl2);

  /* [2] 向量指令序列：装入 / 加法 / 存储，LMUL=2 覆盖 8 个元素 */
  c0 = rdc(); i0 = rdi();
  asm volatile("vsetvli zero, %0, e32, m2, ta, ma" :: "r"((uint64_t)N) : "memory");
  asm volatile("vle32.v v2, (%0)" :: "r"(va) : "memory");
  asm volatile("vle32.v v4, (%0)" :: "r"(vb) : "memory");
  asm volatile("vadd.vv v6, v2, v4" ::: "memory");
  asm volatile("vse32.v v6, (%0)" :: "r"(vc) : "memory");
  asm volatile("fence" ::: "memory");
  c1 = rdc(); i1 = rdi();

  printf("[2] vsetvli zero,%d,e32,m2,ta,ma ; vle32.v v2<-va ; vle32.v v4<-vb\n", N);
  printf("    vadd.vv v6,v2,v4 ; vse32.v v6->vc ; fence\n");
  printf("    该段 retires %d 条指令, 耗时 %d 周期 (含 vsetvli 后的向量流水)\n\n",
         (int)(i1-i0), (int)(c1-c0));

  /* [3] 写回结果 */
  print_vec("[3] 输入 va", va, N);
  print_vec("    输入 vb", vb, N);
  print_vec("    向量结果 vc", vc, N);

  /* [4] 标量参考 + 校验 */
  for (i = 0; i < N; i++) vref[i] = va[i] + vb[i];
  print_vec("[4] 标量参考", vref, N);
  ok = 1;
  for (i = 0; i < N; i++) if (vc[i] != vref[i]) ok = 0;
  printf("    vadd.vv 与标量加法一致性: %s\n\n", ok ? "PASS" : "FAIL");

  /* [5] vmv.x.s 逐 lane 回读（v6 为 LMUL=2 的寄存器对 v6/v7）*/
  {
    int32_t e0, e1;
    asm volatile("vsetivli zero, %0, e32, m2, ta, ma" :: "i"(N) : "memory");
    asm volatile("vmv.x.s %0, v6" : "=r"(e0));
    asm volatile("vslidedown.vx v8, v6, %0" :: "r"((uint64_t)7) : "memory");
    asm volatile("vmv.x.s %0, v8" : "=r"(e1));
    printf("[5] vmv.x.s 读 v6 lane0 = %d (期望 %d)；vslidedown 后读 lane0 = %d (期望 %d)\n",
           (int)e0, (int)vref[0], (int)e1, (int)vref[7]);
  }

  /* [6] 与标量循环的周期对比 */
  {
    uint64_t s0, s1;
    volatile int32_t acc = 0;
    s0 = rdc();
    for (i = 0; i < N; i++) acc = acc + va[i] + vb[i];
    s1 = rdc();
    printf("\n[6] 标量循环累加 %d 个元素: %d 周期, 结果 %d\n", N, (int)(s1-s0), (int)acc);
    printf("    向量一条 vadd.vv 完成同样的 %d 个元素加法: %d 周期（含两条 vle32.v + 一条 vse32.v）\n",
           N, (int)(c1-c0));
  }

  printf("\n=== 演示结束 (cycleCnt/IPC 见 emu 统计) ===\n");
  return 0;
}
