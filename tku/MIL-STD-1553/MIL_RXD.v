`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    21:00:01 04/03/2016 
// Design Name: 
// Module Name:    MIL_RXD 
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
module MIL_RXD( input In_P, output wire ok_SY, 			//Есть синхроимпульс
                input In_N,	output wire dRXP, 			//Импульсы перепадов RXP
                input clk,	output wire[5:0] cb_tact, 	//Счетчик такта
                            output wire[4:0] cb_bit, 	//Счетчик бит
                            output wire en_wr, 			//Интервал разрешения коррекции
                            output wire ce_tact, 		//Границы такта
                            output wire ce_bit,		 	//Центр такта
                            output wire en_rx, 			//Интервал приема слова
                            output wire T_dat, 			//Интервал данных
                            output wire T_end, 			//Интервал контрольного бита
                            output wire FT_cp, 			//Однобитный счетчик единиц
                            output wire [15:0]sr_dat ,  //Регистр сдвига данных
                            output wire ok_rx, 			//Подтверждение верного приема
                            output wire CW_DW);         //Назначение принятых данных 

    parameter RXvel     = 1000000;                      // 1MHz
    parameter Fclk      = 50000000;                     // 50 MHz
    parameter ref_SY    = 70;

    reg[5:0] cbTact     = 0;
    reg[4:0] cbBit      = 0;

    assign RXN          = In_N;
    assign RXP          = In_P;
    assign cb_tact      = cbTact;
    assign cb_bit       = cbBit[4:0];

    // Detecting data word	
    reg [74:0]bufN      = 0;
    reg [6:0]cb_SY_DW   = 0;
    reg [6:0]cb_SY_CW   = 0;
    reg _cw_dw          = 0;

    assign ok_SY_CW		= (cb_SY_CW >= ref_SY);         // current word is control
    assign ok_SY_DW		= (cb_SY_DW	>=	ref_SY);
    assign ok_SY        = (ok_SY_CW | ok_SY_DW);        // currrent word is data
    assign CW_DW        = _cw_dw;
    assign D_RXN        = bufN[74];
    assign Neg_detect 	= RXP & D_RXN;

    always @( posedge clk ) begin
        bufN        <=  (bufN << 1) + RXN;
        cb_SY_DW    <=  Neg_detect  ? (cb_SY_DW + 1): 0;
        _cw_dw 	    <=  ok_rx       ? 0             : 
                        (ok_SY_CW)  ? 1             : 
                        ok_SY_DW    ? 0             : _cw_dw;
    end

    // Detecting control word
    reg [74:0]bufP      = 0;

    assign D_RXP        = bufP[74];
    assign Pos_detect   = RXN & D_RXP;

    always @( posedge clk ) begin
        bufP        <= (bufP << 1) + RXP;
        cb_SY_CW    <= Pos_detect ? (cb_SY_CW + 1) : 0;
    end

    // recieve data and clk synchronization
    parameter set_M     = 25;
    parameter  refL     = 21;
    parameter  refH     = 28;

    reg [15:0] data     = 0;
    reg        tDat		= 0;
    reg			 FP		= 0;
    reg			 OK		= 0;

    assign	T_dat       = tDat;
    assign	T_end       = (cbBit == 16);
    assign 	sr_dat      = data[15:0];
    assign	en_wr       = (cb_tact >= refL) & (cb_tact <= refH);
    assign  ce_tact     = (cbTact == 49);
    assign  dRXP        = bufP[0] ^ bufP[1];
    assign  ce_bit      = dRXP & en_wr;
    assign	FT_cp       = FP;
    assign 	ok_rx       = OK;
    assign 	en_rx       = (T_dat | T_end);

    always @( posedge clk ) begin
        tDat    <= ok_SY  ? 1       : (cbBit == 16)   ? 0                   : tDat;
        cbTact  <= ce_bit ? set_M   : (ce_tact|ok_SY) ? 0                   : cbTact + 1;
        cbBit   <= ok_SY  ? 0		: ce_tact         ? cbBit + 1           : (cbBit == 17) ? 0 : cbBit;

        data    <= ok_SY  ? 0		: (ce_bit & T_dat)? ((data << 1) | RXN) : data;
        FP      <= ok_SY  ? 0		: (ce_bit & T_dat)? (FP ^ RXN) 			: FP;

        OK      <= (ok_SY|OK)  ? 0  : (T_end & ce_bit)? (FP == RXP)			: OK; 
    end	
	
endmodule
