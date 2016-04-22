`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    04:53:36 04/21/2016 
// Design Name: 
// Module Name:    Gen_txen_DAT 
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
module Gen_txen_DAT(input st,   output reg txen=0,
                    input clk,  output wire[15:0] DAT,
                                output wire[15:0] CW_TX,
                                output wire[15:0] DW_TX);

    assign CW_TX    = 16'hDEF0; // my CW (Таблица 1)
    assign DW_TX    = 16'h2233; // my ВW (Таблица 1)

    reg [13:0]cb_txen=0;

    wire ce_end     = (cb_txen == 2200); //20ns*1100=22 000ns=22us>20us
    assign dat_en   = (cb_txen <= 1100);
    assign DAT 	    = dat_en ? CW_TX : DW_TX;

    always @ (posedge clk) begin
        txen    <= st? 1 : ce_end   ? 0         : txen;
        cb_txen <= st? 0 : txen     ? cb_txen+1 : cb_txen;
    end

endmodule