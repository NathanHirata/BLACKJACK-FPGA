`timescale 1ns / 1ps

module title_rom
#(
    parameter ADDR_WIDTH = 15,  // number of address bits
              DATA_WIDTH = 1    // color depth
   )
   (
    input  logic clk,
    input  logic [ADDR_WIDTH-1:0] addr_r,
    output logic [DATA_WIDTH-1:0] dout
   );

   // signal declaration
   logic [DATA_WIDTH-1:0] ram [0:2**ADDR_WIDTH-1];
   logic [DATA_WIDTH-1:0] data_reg;
   
   // ghost_bitmap.txt specifies the initial values of ram 
   initial 
      $readmemb("bkbits.txt", ram);
      
   // body
   always_ff @(posedge clk)
   begin
      data_reg <= ram[addr_r];
   end
   assign dout = data_reg;
endmodule
