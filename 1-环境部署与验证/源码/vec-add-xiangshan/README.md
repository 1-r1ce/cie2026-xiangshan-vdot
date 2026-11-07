# vec-add-xiangshan —— 向量加法指令实测程序（S1-3 配套）

在"香山"昆明湖 V2 微架构上运行 `vadd.vv`，输出 vsetvli 探测、向量加法结果、与标量实现的
一致性校验、以及周期数对比。产出文档见 `1-环境部署与验证/1.3-向量加法指令执行过程.md`，
运行日志见 `1-环境部署与验证/日志/向量加法运行日志.log`。

## 部署到服务器

```bash
source /home/rv/work/xs-build-env.sh
cp -r <本目录> $AM_HOME/apps/vec-add-xiangshan
cd $AM_HOME/apps/vec-add-xiangshan
make ARCH=riscv64-xs MARCH=rv64gcv CROSS_COMPILE=riscv64-linux-gnu-
```

## 运行

```bash
cd /home/rv/work/xs-env/XiangShan
./build/emu --no-diff \
  -i $AM_HOME/apps/vec-add-xiangshan/build/vec-add-xiangshan-riscv64-xs.bin \
  -C 400000 --force-dump-result
```

实测：`instrCnt = 40,691`、`cycleCnt = 116,764`、Host time 181–342 s（约 340–640 周期/秒）。

## 三个必须注意的点

1. **`mstatus.VS` 必须先置为 Initial**（`csrs mstatus, 0x600`）。裸机程序默认 VS=Off，
   此时向量指令既不提交也不报异常，表现为仿真器静默停摆（`instrCnt` 冻结）。
2. `MARCH=rv64gcv` 要显式传给 make：nexus-am 的 `riscv64-xs.mk` 默认 `rv64gc`，不带 V，
   汇编器会拒绝 `vadd.vv`。
3. `--no-diff` 关闭 difftest：参考模型 `riscv64-nemu-interpreter-so` 对向量指令支持不完整，
   开启会误报不匹配。

## 数据布局

`VLEN=128`、`SEW=32`：`vsetvli e32,m1 → vl=4`；要一次处理 8 个元素必须用 `LMUL=2`（`v2/v3`、
`v4/v5`、`v6/v7` 各为 256 bit 寄存器组）。数组按 16 字节对齐，满足 `vle32.v/vse32.v`。
