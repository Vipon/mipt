`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    04:59:15 04/21/2016 
// Design Name: 
// Module Name:    Sch_LAB406AD 
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
module Sch_LAB406AD(	input F50MHz,	output wire JB1,
							input JC1,		output wire JB2,
							input JC2,		output wire JB3,
							input [4:0]SW,	output wire JB4,
							input BTN0,		output wire JB7,
												output wire LED7,
												output wire JB8,
												output wire JA8,
												output wire LED0,
												output wire JA1,
												output wire JA2,
												output wire JA3,
												output wire JA4,
												output wire JA7,
												output wire seg_P,
												output wire [3:0]AN,
												output wire [6:0]seg	);
wire ce1ms,txen;
wire clk;
BUFG DD1 (.I(F50MHz), .O(clk)	);


wire [15:0]DAT, CW_TX, DW_TX;
Gen_txen_DAT	DD2	(	.st(ce1ms),	.txen(txen),
								.clk(clk),	.DAT(DAT),
												.CW_TX(CW_TX),
												.DW_TX(DW_TX)	);
												
MIL_TXD	DD3	(	.clk(clk),			.TXP(JB1),
						.txen(txen),		.TXN(JB2),
						.dat(DAT),			.SY1(JB3),
												.SY2(JB4),
												.en_tx(JB7),
												.ce_tact(JB8)	);
											
BUF	DD4	(	.I(JB7),	.O(LED7)	);


wire In_P, In_N;
M2_1 DD5		(	.D0(JB1),	.O(In_P),
					.D1(JC1),
					.S0(SW[4])					);
					
M2_1 DD6		(	.D0(JB2),	.O(In_N),
					.D1(JC2),
					.S0(SW[4])					);	
					
wire ok_rx;
wire [15:0]sr_dat;
wire CW_DW;				
MIL_RXD	DD7	(	.clk(clk),		.ce_tact(JA8),
						.In_P(In_P),	.en_rx(LED0),
						.In_N(In_N),	.en_wr(JA1),
											.ce_bit(JA2),
											.T_dat(JA3),
											.T_end(JA4),
											.FT_cp(JA7),
											.ok_rx(ok_rx),
											.sr_dat(sr_dat),
											.CW_DW(CW_DW)		);

wire [15:0]CW_RX,DW_RX;											
BUF_RX_DAT	DD8	(	.ce(ok_rx),				.DAT_CW(CW_RX),
							.clk(clk),				.DAT_DW(DW_RX),
							.DAT_RX(sr_dat),
							.CW_DW(CW_DW),
							.R(BTN0)										);

wire [15:0]ODAT;
MUX16_4_1	DD9	(	.A(CW_TX),			.E(ODAT),
							.B(DW_TX),		
							.C(CW_RX),
							.D(DW_RX),
							.adr(SW[1:0])							);
							
DISPLAY	DD10		(	.clk(clk),			.seg_P(seg_P),
							.dat(ODAT),			.ce1ms(ce1ms),
							.ptr_P(SW[3:2]),	.AN(AN),
													.seg(seg)		);
endmodule