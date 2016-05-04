`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    21:26:42 04/25/2016 
// Design Name: 
// Module Name:    Sch_LAB404 
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
module Sch_LAB404(	input F50MHz,		output wire JB1,
							input [7:0]SW,		output wire TXD,
							input RXD,			output wire JB8,
							input BTN3,			output wire LED7,
							input BTN0,			output wire JB2,
													output wire JB3,
													output wire JB4,
													output wire JB7,
													output wire seg_P,
													output wire [3:0]AN,
													output wire [6:0]seg,
													output wire LED0,
													output wire JC2,
													output wire JC4,
													output wire JC8,
													output wire JC3,
													output wire JC7,
													output wire JC1
													);
													
													
wire clk;
wire ce1ms;
wire bf_TXD;
wire Inp;
wire en_tx_byte, en_rx_byte;
wire ok_rx_byte;
wire [7:0]D, RX_DAT;

BUFG DD1 (.I(F50MHz), .O(clk) );

UTXD1B DD2 	(	.clk(clk),		.TXD(TXD),
					.st(ce1ms),		.ce_tact(JB8),
					.dat(SW),		.en_tx_byte(en_tx_byte),
										.T_start(JB3),
										.T_dat(JB4),
										.T_stop(JB7) );
										
M2_1 DD3 	(	.A(RXD),		.Q(Inp),								
					.B(bf_TXD),							
					.S(BTN3) 				);

URXD1B DD4	(	.Inp(Inp),		.en_rx_byte(en_rx_byte),
					.clk(clk),		.ok_rx_byte(ok_rx_byte),
										.T_dat(JC4),
										.ce_tact(JC8),
										.T_start(JC3),
										.T_stop(JC7),
										.RXD(JC1),
										.sr_dat(D) );
										
FD8RE DD5	(	.clk(clk),				.Q(RX_DAT),
					.CE(ok_rx_byte),
					.D(D),
					.R(BTN0)			);

DISPL DD6	(	.clk(clk),					.AN(AN),
					.dat( {SW,RX_DAT} ),		.seg(seg),
													.seg_P(seg_P),
													.ce1ms(ce1ms) );
BUF DD7 ( .I(TXD), .O(JB1) );
BUF DD8 ( .I(TXD), .O(bf_TXD) );
BUF DD9 ( .I(en_tx_byte), .O(JB2) );
BUF DD10 ( .I(en_rx_byte), .O(JC2) );

assign LED0=en_rx_byte;
assign LED7=en_tx_byte;												

endmodule