`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    19:29:53 04/25/2016 
// Design Name: 
// Module Name:    SOURCE_DAT 
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
module SOURCE_DAT(	output wire [8:0]MASTER_dat,
							output wire [8:0]SLAVE_dat );

assign MASTER_dat = 9'b101111010;	//0x17a
assign SLAVE_dat  = 9'b111011011; 	//0x1db

endmodule