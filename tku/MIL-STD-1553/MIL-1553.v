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
module MIL_1553(	input clk_tx, 		output wire TXP, //"Положительные" импульсы
						input[15:0]dat, 	output wire TXN, //"Отрицательные" импульсы
						input txen, 		output wire SY1, // Первый импульс синхронизации
												output wire SY2, // Второй импульс синхронизации
												//-----Вход и выходы MIL_RXD--------------------
						input clk_rx, 		output wire ok_SY, //Есть синхроимпульс
												output wire dRXP, //Импульсы перепадов RXP
												output wire[5:0] cb_tact, //Счетчик такта
												output wire[4:0] cb_bit, //Счетчик бит
												output wire ce_tact, //Границы такта
												output wire en_rx, //Интервал приема слова
												output wire en_wr, //Интервал разрешения коррекции
												output wire ce_bit, //Центр такта
												output wire T_dat, //Интервал данных
												output wire T_end, //Интервал контрольного бита
												output wire FT_cp, //Однобитный счетчик единиц
												output wire [15:0]sr_dat , //Регистр сдвига данных
												output wire ok_rx, //Подтверждение верного приема
												output wire CW_DW //Назначение принятых данных
												);
	//------Модуль передатчика MIL_TXD ------------------------
	MIL_TXD DD1 ( 	.clk(clk_tx), 	.TXP(TXP), //"Положительные" импульсы
						.dat(dat), 		.TXN(TXN), //"Отрицательные" импульсы
						.txen(txen), 	.SY1(SY1), // Первый импульс синхронизации
											.SY2(SY2)); // Второй импульс синхронизации
	//------Модуль приемника MIL_RXD --------------------------
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
