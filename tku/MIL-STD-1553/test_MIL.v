`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   01:03:59 04/04/2016
// Design Name:   MIL_1553
// Module Name:   J:/MIL-STD-1553/test_MIL.v
// Project Name:  MIL-STD-1553
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: MIL_1553
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module test_MIL;

	// Inputs
	reg clk_tx;
	reg [15:0] dat;
	reg txen;
	reg clk_rx;

	// Outputs
	wire TXP;
	wire TXN;
	wire SY1;
	wire SY2;
	wire ok_SY;
	wire dRXP;
	wire [5:0] cb_tact;
	wire [4:0] cb_bit;
	wire ce_tact;
	wire en_rx;
	wire en_wr;
	wire ce_bit;
	wire T_dat;
	wire T_end;
	wire FT_cp;
	wire [15:0] sr_dat;
	wire ok_rx;
	wire CW_DW;

	// Instantiate the Unit Under Test (UUT)
	MIL_1553 uut (
		.clk_tx(clk_tx), 
		.TXP(TXP), 
		.dat(dat), 
		.TXN(TXN), 
		.txen(txen), 
		.SY1(SY1), 
		.SY2(SY2), 
		.clk_rx(clk_rx), 
		.ok_SY(ok_SY), 
		.dRXP(dRXP), 
		.cb_tact(cb_tact), 
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
		.CW_DW(CW_DW)
	);

	parameter PERIOD = 20; 
	
	always begin clk_tx = 1'b0; #(PERIOD/2) clk_tx = 1'b1; #(PERIOD/2); end
	always begin clk_rx = 1'b0; #(PERIOD/2) clk_rx = 1'b1; #(PERIOD/2); end
	
	initial begin
		// Initialize Inputs
		clk_tx = 0;
		clk_rx = 0;
					
                dat = 0; txen = 0; 			//
		#2000; 	dat = 16'h1234; txen = 1; 	// my CW
		#21000; dat = 16'h5678; txen = 1; 	// my WD
		#20000; dat = 16'h0000; txen = 0;

	end
      
endmodule

