`define MAX(a,b) ((a) > (b) ? (a) : (b))
`define STR(x) `"x`"
`ifdef FOO
  int a = `MAX(1, 2);
`elsif BAR
  `undef MAX
`else
  string s = `STR(hello);
`endif
`pragma protect begin_protected
`pragma protect encoding = (enctype = "base64", line_length = 76, bytes = 4)
`pragma protect data_block
YWJjZA==
`pragma protect end_protected
`timescale 1ns/1ps
`default_nettype none
`line 10 "foo.sv" 0
`__FILE__ `__LINE__
