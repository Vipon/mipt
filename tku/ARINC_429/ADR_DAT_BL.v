`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    10:25:58 03/03/2016 
// Design Name: 
// Module Name:    ADR_DAT_BL 
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
module ADR_DAT_BL(				output wire [1:0] VEL,
										output wire [7:0] ADR,
										output wire [22:0] DAT);
	
	wire [4:0] my_nom = 5'b00100 ; //5'b01101=13 ;
	assign ADR = {3'b100,my_nom} ;
	parameter my_dat = 23'h1ffaab;//23'h112200 ;
	parameter my_VEL = 2'b10 ; // 100 kBod
	wire my_bit_dat=my_dat[13];
	assign DAT={my_dat[22:14],my_bit_dat,my_dat[12:0]} ;
	assign VEL = my_VEL ;
endmodule