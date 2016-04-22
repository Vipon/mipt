`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   17:45:24 04/03/2016
// Design Name:   MIL_TXD
// Module Name:   J:/MIL-STD-1553/test_MIL_TXD.v
// Project Name:  MIL-STD-1553
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: MIL_TXD
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module test_MIL_TXD;

	parameter PERIOD = 20; 
	
	// Inputs
	reg [15:0] dat;
	reg txen;
	reg clk;
	
	// Outputs
	wire TXP;
	wire TXN;
	wire SY1;
	wire SY2;
	wire en_tx;
	wire T_dat;
	wire T_end;
	wire SDAT;
	wire FT_cp;
	wire [4:0] cb_bit;
	wire ce_tact;

	// Instantiate the Unit Under Test (UUT)
	MIL_TXD uut (
		.clk(clk), 
		.TXP(TXP), 
		.dat(dat), 
		.TXN(TXN), 
		.txen(txen), 
		.SY1(SY1), 
		.SY2(SY2), 
		.en_tx(en_tx), 
		.T_dat(T_dat), 
		.T_end(T_end), 
		.SDAT(SDAT), 
		.FT_cp(FT_cp), 
		.cb_bit(cb_bit), 
		.ce_tact(ce_tact)
	);

	initial begin
		// Initialize Inputs
					dat = 0;        txen = 0; 			//
		#2000; 		dat = 16'h1234; txen = 1; 	// my CW
		#21000; 	dat = 16'h5678; txen = 1; 	// my WD
		#20000; 	dat = 16'h0000; txen = 0;
	end
	
	always begin clk = 1'b0; #(PERIOD/2) clk = 1'b1; #(PERIOD/2); end
      
endmodule

