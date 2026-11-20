package cfg;
  localparam int unsigned WIDTH = 16;
  typedef enum logic [2:0] { IDLE, RUN, DONE = 3'd5 } state_e;
  typedef struct packed { logic [WIDTH-1:0] data; state_e st; logic valid; } pkt_t;
  typedef union packed { pkt_t p; logic [$bits(pkt_t)-1:0] raw; } pkt_u;
endpackage
