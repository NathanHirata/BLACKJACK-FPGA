`timescale 1ns / 1ps


module chu_vga_card_core
#(parameter CD = 12,   // color depth
            ADDR_WIDTH = 12,
            KEY_COLOR = 0
   )
   (
    input  logic clk, reset,
    // frame counter
    input  logic [10:0] x, y,
    // video slot interface
    input  logic cs,      
    input  logic write,  
    input  logic [13:0] addr,    
    input  logic [31:0] wr_data,
    // stream interface
    input  logic [11:0] si_rgb,
    output logic [11:0] so_rgb
  );

   // signal delaration
   logic wr_en, wr_reg, wr_bypass, wr_char_ram;
   logic [CD-1:0] osd_rgb;
   logic bypass_reg;
   // body
   // instantiate osd generator
   card_src #(.CD(CD)) cards_src_unit (
      .clk(clk), .x(x), .y(y), .xt(addr[4:0]), .yt(addr[7:5]),
      .ch_in(wr_data[5:0]), .we_ch(wr_char_ram),
      .osd_rgb(osd_rgb));
   // register  
   always_ff @(posedge clk, posedge reset)
      if (reset) begin
         bypass_reg <= 0;
      end   
      else begin
         if (wr_bypass)
            bypass_reg <= wr_data[0];
      end      
   // decoding 
   assign wr_en = write & cs;
   assign wr_char_ram = ~addr[13] && wr_en;
   assign wr_reg = addr[13] && wr_en;
   assign wr_bypass   = wr_reg && (addr[1:0]==2'b00);
   // chrome-key blending and multiplexing
   assign so_rgb = (bypass_reg || osd_rgb==KEY_COLOR) ? si_rgb : osd_rgb;
endmodule
