`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    07:17:44 04/21/2016 
// Design Name: 
// Module Name:    SPI_SLAVE 
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
module SPI_SLAVE(		input SCLK,			output wire MISO,
							input LOAD,			output wire [8:0]sr_STX,
							input MOSI,			output wire [8:0]sr_SRX,
							input clr,			output wire [8:0]DO,
							input [8:0]DI   );

parameter m		= 9;

reg [8:0]srSRX = 0;
reg [8:0]srSTX = 0;
reg [8:0]_Do	= 0;

assign sr_STX	= srSTX;
assign sr_SRX	= srSRX;
assign DO		= _Do;
assign MISO		= sr_STX[m-1];

always @(posedge SCLK) begin
	srSRX 	<= !LOAD	? ((srSRX << 1)| MOSI)	: srSRX;
end

always @(posedge LOAD or posedge clr) begin
	_Do 		<=	clr	? 0							: srSRX;
end

always @(posedge LOAD or negedge SCLK) begin
	srSTX 	<= LOAD	? DI							: srSTX << 1;
end

endmodule