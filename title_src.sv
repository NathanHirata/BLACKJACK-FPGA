`timescale 1ns / 1ps


module title_src
#(
    parameter CD = 12,      // color depth
              ADDR = 15,    // number of address bits
              KEY_COLOR =0  // chroma key
   )
   (
    input  logic clk,
    input  logic [10:0] x, y,   // x-and  y-coordinate    
    input  logic [10:0] x0, y0, // origin of sprite 
    // sprite ram write 
    // pixel output
    output logic [CD-1:0] sprite_rgb
   );
   
   // localparam declaration
   localparam H_SIZE = 512; // horizontal size of sprite
   localparam V_SIZE = 64; // vertical size of sprite
   // signal delaration
   logic signed [11:0] xr, yr;  // relative x/y position
   logic in_region;
   logic [ADDR-1:0] addr_r;
   logic plt_code;        
   logic frame_tick, ani_tick;
   logic [5:0] c_next;        
   logic [5:0] c_reg;        
   logic [2:0] ani_next;        
   logic [2:0] ani_reg;        
   logic [10:0] x_d1_reg;
   logic [CD-1:0]  out_rgb;
   logic [CD-1:0] full_rgb, ghost_rgb;
   logic [CD-1:0] out_rgb_d1_reg;
   
   // body 

   //******************************************************************
   // sprite RAM
   //******************************************************************
   // instantiate sprite RAM
   title_rom #(.ADDR_WIDTH(ADDR), .DATA_WIDTH(1)) title_unit (
      .clk(clk), .addr_r(addr_r), .dout(plt_code));
   assign addr_r = {yr[5:0], xr[8:0]};
 
   //******************************************************************
   // ghost color control
   //******************************************************************
   // ghost color selection
   always_comb
      case (ani_reg)
         3'b000:   ghost_rgb = 12'hf00;  // red 
         3'b001:   ghost_rgb = 12'hf8b;  // pink 
         3'b010:   ghost_rgb = 12'hfa0;  // orange
         3'b011:   ghost_rgb = 12'h00b;  // 
         3'b100:   ghost_rgb = 12'h0bb;  // 
         3'b101:   ghost_rgb = 12'hEE0;  // 
         3'b110:   ghost_rgb = 12'ha2c;  // 
         3'b111:   ghost_rgb = 12'h777;  // 
         default: ghost_rgb = 12'hfff;  // white
      endcase   
   // palette table
   assign full_rgb = (plt_code) ? 12'h000 : ghost_rgb;
   //******************************************************************
   // in-region circuit
   //******************************************************************
   // relative coordinate calculation
   assign xr = $signed({1'b0, x}) - $signed({1'b0, x0});
   assign yr = $signed({1'b0, y}) - $signed({1'b0, y0});
   // in-region comparison and multiplexing 
   assign in_region = ((0<= xr) && (xr<H_SIZE) && (0<=yr) && (yr<V_SIZE));
   assign out_rgb = in_region ? full_rgb : KEY_COLOR;
   //******************************************************************
   // animation timing control
   //******************************************************************
   // counters 
   always_ff @(posedge clk) begin
      x_d1_reg <= x;
      c_reg <= c_next;
      ani_reg <= ani_next;
   end
   assign c_next = (frame_tick && c_reg==49) ? 0 :
                   (frame_tick) ? c_reg + 1 :
                    c_reg; 
   assign ani_next = (ani_tick) ? ani_reg + 1 : ani_reg;
   // 60-Hz tick from fram counter 
   assign frame_tick = (x_d1_reg==0) && (x==1) && (y==0);
   // sprite animation id tick 
   assign ani_tick  = frame_tick  && (c_reg==0); 
   // sprite id selection
   //******************************************************************
   // delay line (one clock) 
   //******************************************************************
   // output with a-stage delay line
   always_ff @(posedge clk) 
      out_rgb_d1_reg <= out_rgb;
   assign sprite_rgb = out_rgb_d1_reg;
endmodule