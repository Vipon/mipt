`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    21:29:36 04/25/2016 
// Design Name: 
// Module Name:    FD8RE 
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
module FD8RE(		input clk,			output reg [7:0]Q=0,
						input [7:0]D,
						input CE,
						input R
    );
	 
always @(posedge clk) begin
Q <= R? 0: CE? D: Q;
end

endmodule
