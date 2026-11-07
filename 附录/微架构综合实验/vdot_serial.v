// 变体 A：34 bit 串行累加（对应原 RTL 实现 pad(32) + reduce(_ +& _)）
module vdot_serial(input [127:0] vs1, input [127:0] vs2, output [33:0] sum);
  wire signed [15:0] p0 = $signed(vs1[7:0])   * $signed(vs2[7:0]);
  wire signed [15:0] p1 = $signed(vs1[15:8])  * $signed(vs2[15:8]);
  wire signed [15:0] p2 = $signed(vs1[23:16]) * $signed(vs2[23:16]);
  wire signed [15:0] p3 = $signed(vs1[31:24]) * $signed(vs2[31:24]);
  wire signed [15:0] p4 = $signed(vs1[39:32]) * $signed(vs2[39:32]);
  wire signed [15:0] p5 = $signed(vs1[47:40]) * $signed(vs2[47:40]);
  wire signed [15:0] p6 = $signed(vs1[55:48]) * $signed(vs2[55:48]);
  wire signed [15:0] p7 = $signed(vs1[63:56]) * $signed(vs2[63:56]);
  wire [33:0] s0 = {{18{p0[15]}}, p0};
  wire [33:0] s1 = {{18{p1[15]}}, p1};
  wire [33:0] s2 = {{18{p2[15]}}, p2};
  wire [33:0] s3 = {{18{p3[15]}}, p3};
  wire [33:0] s4 = {{18{p4[15]}}, p4};
  wire [33:0] s5 = {{18{p5[15]}}, p5};
  wire [33:0] s6 = {{18{p6[15]}}, p6};
  wire [33:0] s7 = {{18{p7[15]}}, p7};
  wire [33:0] t1 = s0 + s1;
  wire [33:0] t2 = t1 + s2;
  wire [33:0] t3 = t2 + s3;
  wire [33:0] t4 = t3 + s4;
  wire [33:0] t5 = t4 + s5;
  wire [33:0] t6 = t5 + s6;
  wire [33:0] t7 = t6 + s7;
  assign sum = t7;
endmodule