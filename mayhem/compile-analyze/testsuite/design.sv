package p; typedef struct packed { logic [3:0] a; enum logic [1:0] {X, Y} e; } s_t; localparam int W = 8; endpackage
module top #(parameter int N = 4) (input logic clk, input logic [N-1:0] d, output logic [N-1:0] q);
  `define INC(x) ((x) + 1)
  logic [N-1:0] r;
  always_ff @(posedge clk) r <= d;
  always_comb q = `INC(r);
  function automatic int f(int x); return x <= 1 ? 1 : x * f(x - 1); endfunction
  localparam int F = f(5);
  int qq[$]; initial begin qq.push_back(1); $display("%0d %s", qq[0], $sformatf("%h", F)); end
  for (genvar i = 0; i < 2; i++) begin : g sub u(.a(d[i])); end
endmodule
module sub(input logic a); endmodule
class C; rand int x; constraint c { x > 0; } endclass
