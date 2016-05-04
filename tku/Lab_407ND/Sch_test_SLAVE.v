`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    07:31:37 04/21/2016 
// Design Name: 
// Module Name:    Sch_test_SLAVE 
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
`define m 9 // m из Таблицы 1
module Sch_test_SLAVE(
//--------------Выводы SPI_MASTER----------------------
								input st, 					output wire LOAD,
								input clk, 					output wire SCLK,
								input[`m-1:0]MTX_DAT, 	output wire MOSI, 
								input RESET, 				output wire [`m-1:0]MRX_DAT,
																output wire [`m-1:0]sr_MTX,
																output wire [`m-1:0]sr_MRX,
																output wire [7:0]cb_bit,
																output wire ce_tact,
//-------------Выводы SPI_SLAVE---------------------- 
								input [`m-1:0]STX_DAT, 	output wire MISO,
																output wire[`m-1:0]sr_STX,
																output wire[`m-1:0]sr_SRX,
																output wire[`m-1:0]SRX_DAT);
SPI_MASTER DD1 (	.st(st), 		.LOAD(LOAD),
						.clk(clk), 		.SCLK(SCLK),
						.DI(MTX_DAT),	.MOSI(MOSI),
						.clr(RESET), 	.DO(MRX_DAT),
						.MISO(MISO), 	.sr_MTX(sr_MTX),
											.sr_MRX(sr_MRX),
											.cb_bit(cb_bit),
											.ce_tact(ce_tact));
											
SPI_SLAVE DD2 ( 	.LOAD(LOAD), 	.MISO(MISO),
						.SCLK(SCLK), 	.sr_STX(sr_STX),
						.MOSI(MOSI), 	.sr_SRX(sr_SRX),
						.clr(RESET), 	.DO(SRX_DAT),
						.DI(STX_DAT));
						
endmodule
