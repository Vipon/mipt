`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   11:43:01 04/21/2016
// Design Name:   MIL_RXD
// Module Name:   G:/MIL-STD-1553/test.v
// Project Name:  MIL-STD-1553
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: MIL_RXD
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module test;
    // Inputs
    wire In_P;
    wire In_N;
    reg clk_tx;
    reg clk_rx;

    parameter PERIOD = 20; 
    always begin clk_tx = 1'b0; #(PERIOD/2) clk_tx = 1'b1; #(PERIOD/2); end
    always begin clk_rx = 1'b0; #(PERIOD/2) clk_rx = 1'b1; #(PERIOD/2); end
    
    // Outputs
    wire ok_SY;
    wire dRXP;
    wire [5:0] cb_tact;
    wire [4:0] cb_bit;
    wire en_wr;
    wire ce_tact;
    wire ce_bit;
    wire en_rx;
    wire T_dat;
    wire T_end;
    wire FT_cp;
    wire [15:0] sr_dat;
    wire ok_rx;
    wire CW_DW;

    reg st = 0;
    wire txen;
    wire [15:0]DAT;
    Gen_txen_DAT _DAT(  .st(st),        .txen(txen),
                        .clk(clk_tx),   .DAT(DAT),
                                        .CW_TX(),
                                        .DW_TX());

    MIL_TXD DD1 ( 	.clk(clk_tx), 	.TXP(In_P), //"Положительные" импульсы
                    .dat(DAT), 		.TXN(In_N), //"Отрицательные" импульсы
                    .txen(txen), 	.SY1(SY1),  // Первый импульс синхронизации
                    .SY2(SY2));                 // Второй импульс синхронизации
                    
    // Instantiate the Unit Under Test (UUT)
    MIL_RXD uut (   .In_P(In_P), 
                    .ok_SY(ok_SY), 
                    .In_N(In_N), 
                    .dRXP(dRXP), 
                    .clk(clk_rx), 
                    .cb_tact(cb_tact), 
                    .cb_bit(cb_bit), 
                    .en_wr(en_wr), 
                    .ce_tact(ce_tact), 
                    .ce_bit(ce_bit), 
                    .en_rx(en_rx), 
                    .T_dat(T_dat), 
                    .T_end(T_end), 
                    .FT_cp(FT_cp), 
                    .sr_dat(sr_dat), 
                    .ok_rx(ok_rx), 
                    .CW_DW(CW_DW));

    initial begin
        // Initialize Inputs
                st = 0; 
        #2000; 	st = 1; 
        #20;    st = 0;
        // Add stimulus here

    end

endmodule

