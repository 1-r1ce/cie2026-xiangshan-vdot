#!/bin/bash
# 香山 RTL × 扩展后 NEMU 的协同仿真（difftest）复现脚本
# 用法：在服务器上 source 环境后执行
#   source /home/rv/work/xs-build-env.sh && bash run-cosim.sh
set -e

XS=/home/rv/work/xs-env/XiangShan
APP=$AM_HOME/apps/vdot-xiangshan
MARCHV="rv64gcv_zba_zbb_zbc_zbs_zbkb_zbkc_zbkx_zknd_zkne_zknh_zkr_zksed_zksh_zkt"

# 1) 编译测试程序（vdot 三条用例 + 边界/负数/饱和）
cd $APP
make clean >/dev/null 2>&1 || true
make ARCH=riscv64-xs CROSS_COMPILE=riscv64-linux-gnu- MARCH=$MARCHV
BIN=$APP/build/vdot-xiangshan-riscv64-xs.bin

# 2) 确认参考模型是我们扩展后的版本（1,510,408 B）
ls -l $XS/ready-to-run/riscv64-nemu-interpreter-so

# 3) 协同仿真：RTL 每提交一条指令，NEMU 同步执行并比对架构状态
cd $XS
./build/emu -i $BIN -C 200000 --diff ready-to-run/riscv64-nemu-interpreter-so
# 期望输出：The first instruction of core 0 has commited. Difftest enabled.
#           5/5 用例 PASS；instrCnt = 21,500，cycleCnt = 74,585，IPC = 0.288262
#           无 "different at pc" / "ABORT" 字样