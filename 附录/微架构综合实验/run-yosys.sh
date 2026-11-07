#!/bin/bash
# 复现《微架构优化分析.md》的四个变体综合数据
#   yosys 0.33（apt install yosys），ABC 面积导向综合，取单元数与最长拓扑路径
# 注意：不要加 -q，否则 stat 的输出也会被吞掉
set -u
cd "$(dirname "$0")"
for m in vdot_serial vdot_tree vdot_serial19 vdot_tree19; do
  echo "=== $m ==="
  yosys -p "read_verilog $m.v; synth -top $m; stat; ltp -noff" 2>/dev/null \
    | sed -n '/Printing statistics/,$p' \
    | grep -E "Number of cells|Longest topological"
done