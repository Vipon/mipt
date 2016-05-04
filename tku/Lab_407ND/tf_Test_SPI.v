`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   07:35:23 04/21/2016
// Design Name:   Sch_test_SLAVE
// Module Name:   J:/Lab_407ND/tf_Test_SPI.v
// Project Name:  Lab_407ND
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: Sch_test_SLAVE
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module tf_Test_SPI;

	// Inputs
	reg st;
	reg clk;
	reg [8:0] MTX_DAT;
	reg RESET;
	reg [8:0] STX_DAT;

	// Outputs
	wire LOAD;
	wire SCLK;
	wire MOSI;
	wire [8:0] MRX_DAT;
	wire [8:0] sr_MTX;
	wire [8:0] sr_MRX;
	wire [7:0] cb_bit;
	wire ce_tact;
	wire MISO;
	wire [8:0] sr_STX;
	wire [8:0] sr_SRX;
	wire [8:0] SRX_DAT;

	// Instantiate the Unit Under Test (UUT)
	Sch_test_SLAVE uut (
		.st(st), 
		.LOAD(LOAD), 
		.clk(clk), 
		.SCLK(SCLK), 
		.MTX_DAT(MTX_DAT), 
		.MOSI(MOSI), 
		.RESET(RESET), 
		.MRX_DAT(MRX_DAT), 
		.sr_MTX(sr_MTX), 
		.sr_MRX(sr_MRX), 
		.cb_bit(cb_bit), 
		.ce_tact(ce_tact), 
		.STX_DAT(STX_DAT), 
		.MISO(MISO), 
		.sr_STX(sr_STX), 
		.sr_SRX(sr_SRX), 
		.SRX_DAT(SRX_DAT)
	);
always begin clk = 0; #10; clk = 1; #10; end
	initial begin
		// Initialize Inputs
		st = 0;
		MTX_DAT = 9'b101111010;
		RESET = 0;
		STX_DAT = 9'b111011011;

		#1000;
      st = 1;
		#20;
		st = 0;
		#20000;
				
		RESET = 1;
		#20;
		RESET = 0;
	end
      
endmodule

