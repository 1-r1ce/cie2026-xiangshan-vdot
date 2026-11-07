// 变体 D：19 bit Wallace 压缩树（采用版本，与 VIPU.scala 的 vdotCsa 段逐级对应）
module vdot_tree19(input [127:0] vs1, input [127:0] vs2, output [31:0] sum);
  wire signed [15:0] p0 = $signed(vs1[7:0])   * $signed(vs2[7:0]);
  wire signed [15:0] p1 = $signed(vs1[15:8])  * $signed(vs2[15:8]);
  wire signed [15:0] p2 = $signed(vs1[23:16]) * $signed(vs2[23:16]);
  wire signed [15:0] p3 = $signed(vs1[31:24]) * $signed(vs2[31:24]);
  wire signed [15:0] p4 = $signed(vs1[39:32]) * $signed(vs2[39:32]);
  wire signed [15:0] p5 = $signed(vs1[47:40]) * $signed(vs2[47:40]);
  wire signed [15:0] p6 = $signed(vs1[55:48]) * $signed(vs2[55:48]);
  wire signed [15:0] p7 = $signed(vs1[63:56]) * $signed(vs2[63:56]);
  wire [18:0] s0 = {{3{p0[15]}}, p0};
  wire [18:0] s1 = {{3{p1[15]}}, p1};
  wire [18:0] s2 = {{3{p2[15]}}, p2};
  wire [18:0] s3 = {{3{p3[15]}}, p3};
  wire [18:0] s4 = {{3{p4[15]}}, p4};
  wire [18:0] s5 = {{3{p5[15]}}, p5};
  wire [18:0] s6 = {{3{p6[15]}}, p6};
  wire [18:0] s7 = {{3{p7[15]}}, p7};
  function [18:0] csa_s(input [18:0] a, input [18:0] b, input [18:0] c);
    csa_s = a ^ b ^ c;
  endfunction
  function [18:0] csa_c(input [18:0] a, input [18:0] b, input [18:0] c);
    reg [18:0] m;
    begin m = (a & b) | (a & c) | (b & c); csa_c = {m[17:0], 1'b0}; end
  endfunction
  wire [18:0] s01 = csa_s(s0, s1, s2);
  wire [18:0] c01 = csa_c(s0, s1, s2);
  wire [18:0] s0123 = csa_s(s01, c01, s3);
  wire [18:0] c0123 = csa_c(s01, c01, s3);
  wire [18:0] s456 = csa_s(s4, s5, s6);
  wire [18:0] c456 = csa_c(s4, s5, s6);
  wire [18:0] s4567 = csa_s(s456, c456, s7);
  wire [18:0] c4567 = csa_c(s456, c456, s7);
  wire [18:0] sA = csa_s(s0123, c0123, s4567);
  wire [18:0] cA = csa_c(s0123, c0123, s4567);
  wire [18:0] sB = csa_s(sA, cA, c4567);
  wire [18:0] cB = csa_c(sA, cA, c4567);
  wire [18:0] r = sB + cB;
  assign sum = {{13{r[18]}}, r};
endmodule