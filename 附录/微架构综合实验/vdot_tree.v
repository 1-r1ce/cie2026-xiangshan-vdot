// 变体 B：34 bit Wallace 压缩树（4 级 3:2 CSA + 1 级 CPA）
module vdot_tree(input [127:0] vs1, input [127:0] vs2, output [33:0] sum);
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
  // 3:2 CSA：和 = a^b^c，进位 = maj(a,b,c) 左移一位
  function [33:0] csa_s(input [33:0] a, input [33:0] b, input [33:0] c);
    csa_s = a ^ b ^ c;
  endfunction
  function [33:0] csa_c(input [33:0] a, input [33:0] b, input [33:0] c);
    reg [33:0] m;
    begin m = (a & b) | (a & c) | (b & c); csa_c = {m[32:0], 1'b0}; end
  endfunction
  wire [33:0] s01 = csa_s(s0, s1, s2);
  wire [33:0] c01 = csa_c(s0, s1, s2);
  wire [33:0] s0123 = csa_s(s01, c01, s3);
  wire [33:0] c0123 = csa_c(s01, c01, s3);
  wire [33:0] s4567 = csa_s(s4, s5, s6);
  wire [33:0] c4567 = csa_c(s4, s5, s6);
  wire [33:0] s45677 = csa_s(s4567, c4567, s7);
  wire [33:0] c45677 = csa_c(s4567, c4567, s7);
  wire [33:0] sA = csa_s(s0123, c0123, s45677);
  wire [33:0] cA = csa_c(s0123, c0123, s45677);
  wire [33:0] sB = csa_s(sA, cA, c45677);
  wire [33:0] cB = csa_c(sA, cA, c45677);
  assign sum = sB + cB;
endmodule