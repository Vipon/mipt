`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    21:13:25 04/03/2016 
// Design Name: 
// Module Name:    MIL-1553 
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
module MIL_1553(	input clk_tx, 		output wire TXP, //"�������������" ��������
						input[15:0]dat, 	output wire TXN, //"�������������" ��������
						input txen, 		output wire SY1, // ������ ������� �������������
												output wire SY2, // ������ ������� �������������
												//-----���� � ������ MIL_RXD--------------------
						input clk_rx, 		output wire ok_SY, //���� �������������
												output wire dRXP, //�������� ��������� RXP
												output wire[5:0] cb_tact, //������� �����
												output wire[4:0] cb_bit, //������� ���
												output wire ce_tact, //������� �����
												output wire en_rx, //�������� ������ �����
												output wire en_wr, //�������� ���������� ���������
												output wire ce_bit, //����� �����
												output wire T_dat, //�������� ������
												output wire T_end, //�������� ������������ ����
												output wire FT_cp, //���������� ������� ������
												output wire [15:0]sr_dat , //������� ������ ������
												output wire ok_rx, //������������� ������� ������
												output wire CW_DW //���������� �������� ������
												);
	//------������ ����������� MIL_TXD ------------------------
	MIL_TXD DD1 ( 	.clk(clk_tx), 	.TXP(TXP), //"�������������" ��������
						.dat(dat), 		.TXN(TXN), //"�������������" ��������
						.txen(txen), 	.SY1(SY1), // ������ ������� �������������
											.SY2(SY2)); // ������ ������� �������������
	//------������ ��������� MIL_RXD --------------------------
	MIL_RXD DD2 ( 	.In_P(TXP), 	.ok_SY(ok_SY),
						.In_N(TXN), 	.dRXP(dRXP),
						.clk(clk_rx), 	.cb_tact(cb_tact),
											.cb_bit(cb_bit),
											.ce_tact(ce_tact),
											.en_rx(en_rx),
											.en_wr(en_wr),
											.ce_bit(ce_bit),
											.T_dat(T_dat),
											.T_end(T_end),
											.FT_cp(FT_cp),
											.sr_dat(sr_dat),
											.ok_rx(ok_rx),
											.CW_DW(CW_DW) );
endmodule
