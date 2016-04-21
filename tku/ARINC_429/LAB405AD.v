`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    10:40:37 03/03/2016 
// Design Name: 
// Module Name:    LAB405AD 
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
module LAB405AD(    input F50MHz    /*clk*/,
                    input BTN0      /*st*/,
                    output [3:0] AN,        // �����
                    output [6:0] seg,       // ��������
                    output seg_P,           // �����
                    output ce1mss);

wire clk;

BUFG clkbuf(.I(F50MHz),.O(clk));						

wire [22:0] DAT;
wire [7:0] ADR;
wire [1:0] VEL;

ADR_DAT_BL data(	.VEL(VEL),
                    .ADR(ADR),
                    .DAT(DAT));

wire TXD0;
wire TXD1;

AR_TXD Tran(.clk(clk),  .ce(ce), 				// �������� (Tce=1/Vel)
            .Nvel(VEL), .TXD1(TXD1), 			// �������� ������ 1
            .ADR(ADR),  .TXD0(TXD0), 			// �������� ������ 0
            .DAT(DAT),  .SLP(SLP), 				// �������� �������
            .st(BTN0), 	 .en_tx(), 		        // �������� �������� ������
            .T_cp(), 			                // ���� ������������ ����
            .FT_cp(), 			                // ������� �������� ��������
            .SDAT(), 			                // ���������������� ������
            .QM(), 				                // ���������
            .cb_bit(), 	                        // ������� ���
            .en_tx_word()); 	                // �������� �������� �����

wire [22:0] sr_dat;
wire [7:0] sr_adr;

AR_RXD Resv(.clk(clk),                      // clock
            .in0(TXD0),                     // input for 0 signal
            .in1(TXD1),                     // input for 1 signal
            .sr_dat(sr_dat),                // received data
            .sr_adr(sr_adr),                // received address
            .ce_wr(ce_wr)                   // receive completed correctly
            );

reg set_P = 0;
DISPLAY dis(.clk(clk), 				.AN(AN), //�����
            .dat(sr_dat[15:0]),		.seg(seg), //��������
            .set_P(set_P), 			.seg_P(seg_P), //�����
            .ce1ms(ce1mss));

endmodule
