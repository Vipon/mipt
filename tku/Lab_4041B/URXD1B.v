`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    20:09:32 04/25/2016 
// Design Name: 
// Module Name:    URXD1B 
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
module URXD1B(		input Inp,			output reg en_rx_byte=0,
						input clk,			output reg ok_rx_byte=0,
												output wire start,
												output wire T_dat,
												output wire T_start,
												output wire ce_tact,
												output wire ce_bit,
												output wire T_stop,
												output reg RXD=0,
												output reg [7:0]sr_dat=0,
												output reg [3:0]cb_bit=0    );
parameter Fclk=50000000;
parameter VEL = 38400;
parameter Nt = Fclk/VEL; 

reg [11:0]cb_tact=0; 																	// tact counter
assign ce_tact = (cb_tact == Nt); 												
assign ce_bit 	= (cb_tact == (Nt/2));

assign T_start = (cb_bit == 0) & (!Inp); 													// Receive start signal
assign T_dat 	= (cb_bit < 9) & (cb_bit > 0); 								// Receive data signal
assign T_stop 	= (cb_bit == 9); 													// Receive stop signal
assign ce_stop = T_stop & ce_tact ; 

reg [1:0]aRXD 	= 0; 

assign tRXD 	= aRXD[1];
assign dRXD		= tRXD & (!RXD);	
assign start	= dRXD & (!en_rx_byte);

always @(posedge clk) begin
	RXD			<=	Inp; 	
	aRXD 			<= (aRXD << 1) | Inp;

	cb_tact		<= (start | ce_tact) 	? 0 : cb_tact + 1;
	cb_bit		<= (start | (cb_bit == 9) & ce_tact)? 0 : (en_rx_byte & ce_tact)? cb_bit+1 : cb_bit; 

	en_rx_byte<= (T_start & ce_bit & (RXD==0))? 1: ok_rx_byte? 0: en_rx_byte;
	ok_rx_byte<= (ce_bit & T_stop & (RXD==1) & en_rx_byte);
	sr_dat<= (T_dat & ce_bit)? ( sr_dat>>1 | {RXD,7'b0} ) : sr_dat;
end

endmodule