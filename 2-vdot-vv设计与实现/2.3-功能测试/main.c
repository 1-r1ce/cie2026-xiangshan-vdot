/*
 * CIE-2026 自定义指令 vdot.vv 功能测试（阶段二交付物）
 *
 * 语义（本项目定义）：
 *   vdot.vv vd, vs1, vs2
 *   vd[0](低 32 bit) = Σ_{i=0..7} sext8(vs1[i]) * sext8(vs2[i])   其余位清零
 *
 * 编码：custom-0 opcode = 0x0B，funct3 = 000，funct7 = 0000000
 *       指令字 = funct7(7) | rs2(5) | rs1(5) | funct3(3) | rd(5) | opcode(7)
 *       本例 vd=v3, vs1=v1, vs2=v2：
 *       0<<25 | 2<<20 | 1<<15 | 0<<12 | 3<<7 | 0x0B = 0x0020818B
 *   （binutils 2.42 的 `.insn r` 不接受向量寄存器，故直接写编码字）
 *
 * 覆盖赛题要求的三项测试：正整数 / 负数（含混合符号）/ 边界值。
 *
 * 部署：nexus-am/apps/vdot-xiangshan/main.c
 * 编译：make ARCH=riscv64-xs MARCH=rv64gcv CROSS_COMPILE=riscv64-linux-gnu-
 * 运行：./build/emu --no-diff -i <bin> -C 600000 --force-dump-result
 * 实测：5 个用例全部 PASS（见 ../tests/vdot-run.log）
 */
#include <am.h>
#include <klib.h>
#include <stdint.h>

#define VEC_BYTES 16
static int8_t __attribute__((aligned(16))) sa[VEC_BYTES];
static int8_t __attribute__((aligned(16))) sb[VEC_BYTES];

/* 执行 vdot.vv v3, v1, v2 并回读 vd[0] 的低 32 位 */
static int32_t vdot_vv(void) {
  int32_t out = 0;
  __asm__ volatile(".word 0x0020818B" ::: "memory");  /* vdot.vv v3, v1, v2 : custom-0 */
  /* 用 e32 视图读回 vd[0] 的低 32 bit */
  __asm__ volatile("vsetvli zero, %0, e32, m1, ta, ma" :: "r"((uint64_t)1) : "memory");
  __asm__ volatile("vmv.x.s %0, v3" : "=r"(out));
  return out;
}

/* 标量参考：8 个 int8 元素的点积（32 位有符号） */
static int32_t ref_dot(int8_t *a, int8_t *b) {
  int32_t acc = 0;
  int i;
  for (i = 0; i < 8; i++) acc += (int32_t)a[i] * (int32_t)b[i];
  return acc;
}

static void load_pair(int8_t *a, int8_t *b) {
  int i;
  for (i = 0; i < VEC_BYTES; i++) { sa[i] = a[i]; sb[i] = b[i]; }
  /* 只让前 8 个元素参与点积 */
  __asm__ volatile("vsetvli zero, %0, e8, m1, ta, ma" :: "r"((uint64_t)8) : "memory");
  __asm__ volatile("vle8.v v1, (%0)" :: "r"(sa) : "memory");
  __asm__ volatile("vle8.v v2, (%0)" :: "r"(sb) : "memory");
}

static int run_case(const char *name, int8_t *a, int8_t *b) {
  int32_t got, exp;
  int ok;
  load_pair(a, b);
  got = vdot_vv();
  exp = ref_dot(sa, sb);
  ok = (got == exp);
  printf("  %-14s got = %d, expect = %d -> %s\n", name, (int)got, (int)exp, ok ? "PASS" : "FAIL");
  return ok;
}

int main() {
  uint64_t mst;
  int all = 1;

  printf("=== CIE-2026 custom instruction vdot.vv test ===\n");
  printf("Team: panpan / Qilu Normal University\n");
  printf("semantics: vd[0](32bit) = sum(sext8(vs1[i])*sext8(vs2[i])), i=0..7\n\n");

  /* 向量状态使能：mstatus.VS = Initial，否则向量指令静默停摆 */
  __asm__ volatile("csrs mstatus, %0" :: "r"((uint64_t)0x600) : "memory");
  __asm__ volatile("csrr %0, mstatus" : "=r"(mst));
  printf("[env] mstatus = 0x%x (VS = %d)\n\n", (unsigned)mst, (int)((mst >> 9) & 3));

  /* ---------- 测试 1：正整数 ---------- */
  {
    int8_t a[VEC_BYTES] = {1,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0};
    int8_t b[VEC_BYTES] = {10,20,30,40,50,60,70,80,0,0,0,0,0,0,0,0};
    printf("[test 1] positive integers (expect 2040)\n");
    all &= run_case("positive", a, b);
  }

  /* ---------- 测试 2：负数与混合符号 ---------- */
  {
    int8_t a[VEC_BYTES] = {-1,-2,-3,-4,5,6,-7,-8,0,0,0,0,0,0,0,0};
    int8_t b[VEC_BYTES] = {9,-8,7,-6,-5,4,3,-2,0,0,0,0,0,0,0,0};
    printf("[test 2] negative / mixed sign\n");
    all &= run_case("negative", a, b);
  }

  /* ---------- 测试 3：边界值 ---------- */
  {
    int8_t a[VEC_BYTES] = {127,127,127,127,127,127,127,127,0,0,0,0,0,0,0,0};
    int8_t b[VEC_BYTES] = {127,127,127,127,127,127,127,127,0,0,0,0,0,0,0,0};
    printf("[test 3a] max*max (expect 129032)\n");
    all &= run_case("max*max", a, b);
  }
  {
    int8_t a[VEC_BYTES] = {-128,-128,-128,-128,-128,-128,-128,-128,0,0,0,0,0,0,0,0};
    int8_t b[VEC_BYTES] = {-128,-128,-128,-128,-128,-128,-128,-128,0,0,0,0,0,0,0,0};
    printf("[test 3b] min*min (expect 131072)\n");
    all &= run_case("min*min", a, b);
  }
  {
    int8_t a[VEC_BYTES] = {127,-128,127,-128,127,-128,127,-128,0,0,0,0,0,0,0,0};
    int8_t b[VEC_BYTES] = {-128,127,-128,127,-128,127,-128,127,0,0,0,0,0,0,0,0};
    printf("[test 3c] extreme cancel (expect -130048)\n");
    all &= run_case("mixed-extreme", a, b);
  }

  printf("\n=== summary: %s ===\n", all ? "ALL PASS" : "SOME FAIL");
  return 0;
}
