`timescale 1ns / 1ps


module card_src
#(
    parameter CD = 12,      // color depth
              KEY_COLOR =0  // chroma key
   )
   (
    input  logic clk,
    input  logic [10:0] x, y,   // x-and  y-coordinate    
    // tile ram write port 
    input  logic [4:0] xt,
    input  logic [2:0] yt,
    input  logic [5:0] ch_in, // char data
    input  logic we_ch,       // char write enable
    // pixel output
    output logic [CD-1:0] osd_rgb
   );
   
   // localparam declaration
   localparam NULL_CHAR = 3'b000; 
   // signal delaration
   // font ROM
   logic [3:0] char_addr;
   logic [14:0] rom_addr;
   logic [5:0] row_addr;
   logic [4:0] col_addr;
   //
   logic [2:0] font_word;
   // char tile RAM
   logic [7:0] addr_w, addr_r;
   logic [3:0] ch_ram_out;
   logic [3:0] ch_d1_reg;
   // delay line 
   logic [4:0] x_delay1_reg;
   logic [5:0] y_delay1_reg;
   // other signals
   logic [CD-1:0] f_rgb;
   
     
   // body 
   // *****************************************************************
   // instantiation
   // *****************************************************************
   // instantiate font ROM
   card_rom card_unit (
      .clk(clk), .addr(rom_addr), .data(font_word));
      
   // instantiate dual port tile RAM (2^12-by-8)
   sync_rw_port_ram #(.ADDR_WIDTH(8), .DATA_WIDTH(4)) card_ram_unit ( 
      .clk(clk),
      // write from main system
      .we(we_ch), .addr_w(addr_w), .din(ch_in[3:0]),
      // read to vga
      .addr_r(addr_r), .dout(ch_ram_out)
      );
   // tile RAM write
   assign addr_w = {yt, xt};
   // *****************************************************************
   // delay-line registers
   // ***************************************************************** 
   always_ff @(posedge clk) begin
      y_delay1_reg <= y[5:0];
      x_delay1_reg <= x[4:0];
      ch_d1_reg <= ch_ram_out;
   end
   // *****************************************************************
   // pixel data read
   // *****************************************************************
   // tile RAM address
   assign addr_r = {y[8:6], x[9:5]};
   assign char_addr = ch_ram_out;  // 7 LSBs (ascii code)
   // font ROM
   assign row_addr = y_delay1_reg;  
   assign col_addr = x_delay1_reg;          
   assign rom_addr = {char_addr, row_addr,col_addr};
   // select a bit
   
   always_comb
      case (font_word)
         3'b000:   f_rgb = 12'h000;   // chroma
         3'b001:   f_rgb = 12'h001; // black
         3'b010:   f_rgb = 12'hd23; // red
         3'b011:   f_rgb = 12'hff1; // yellow
         3'b100:   f_rgb = 12'h09d; // blue
         3'b101:   f_rgb = 12'hfca; // tan
         3'b110:   f_rgb = 12'haaa; // silver
         3'b111:   f_rgb = 12'hfff; // white
         default: f_rgb = 12'hfff;  // white
      endcase 
  
   // transparency control
   assign osd_rgb = (ch_d1_reg==NULL_CHAR) ? KEY_COLOR : f_rgb;
endmodule
