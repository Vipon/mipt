`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    19:31:47 04/25/2016 
// Design Name: 
// Module Name:    Gen_st 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module Gen_st(		input clk, input BTN, output wire ce_st   );


parameter Tclk=20;//Tclk = 20 ns;
parameter Trep = 200000;//200us = 200000 ns;
parameter N = Trep/Tclk;

reg [14:0]cb_tact=0;
assign ce_st = (cb_tact == N-1) & BTN;
always @(posedge clk) begin
cb_tact <= ce_st? 0: cb_tact+1;
end

endmodule