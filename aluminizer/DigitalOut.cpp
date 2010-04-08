#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#ifndef _NO_MCC_DIO

#include <cbw.h>

#include "DigitalOut.h"

MCCout::MCCout(int board_num, int port_num, int port_type) :
	board_num(board_num),
	port_num(port_num),
	port_type(port_type),
	num_bits(8),
	state(num_bits, 0)
{
	float RevLevel = (float)CURRENTREVNUM;

	/* Declare UL Revision Level */
	int ULStat = cbDeclareRevision(&RevLevel);

	/* Initiate error handling
	   Parameters:
	      PRINTALL :all warnings and errors encountered will be printed
	      DONTSTOP :program will continue even if error occurs.
	               Note that STOPALL and STOPFATAL are only effective in
	               Windows applications, not Console applications.
	 */
	cbErrHandling(PRINTALL, DONTSTOP);

	/* configure FIRSTPORTA for digital output
	   Parameters:
	      BoardNum    :the number used by CB.CFG to describe this board.
	      PortNum     :the output port
	      Direction   :sets the port for input or output */

	ULStat = cbDConfigPort(board_num, port_num, DIGITALOUT);

	for (size_t i = 0; i < num_bits; i++)
		ULStat = cbDBitOut(board_num, port_num, i, state[i] ? 1 : 0);
}


unsigned MCCout::GetBit(unsigned nBit)
{
	if (nBit < num_bits)
		return state[nBit];
	else
		throw runtime_error("[MCCout::GetBit] no such bit: " + to_string<int>(nBit));
}

void MCCout::SetBit(unsigned nBit, unsigned value)
{
	if (nBit < num_bits)
	{
		if (value != state[nBit])
		{
			state[nBit] = value;
			cbDBitOut(board_num, port_num, nBit, value ? 1 : 0);
		}
	}
}


MCCin::MCCin(int board_num, int port_num, int port_type) :
	board_num(board_num),
	port_num(FIRSTPORTB),
	port_type(port_type),
	num_bits(8)
{
	float RevLevel = (float)CURRENTREVNUM;

	/* Declare UL Revision Level */
	int ULStat = cbDeclareRevision(&RevLevel);

	/* Initiate error handling
	   Parameters:
	      PRINTALL :all warnings and errors encountered will be printed
	      DONTSTOP :program will continue even if error occurs.
	               Note that STOPALL and STOPFATAL are only effective in
	               Windows applications, not Console applications.
	 */
	cbErrHandling(PRINTALL, DONTSTOP);

	/* configure FIRSTPORTA for digital output
	   Parameters:
	      BoardNum    :the number used by CB.CFG to describe this board.
	      PortNum     :the output port
	      Direction   :sets the port for input or output */

	ULStat = cbDConfigPort(board_num, FIRSTPORTB, DIGITALIN);
}


bool MCCin::GetBit(unsigned nBit)
{
	if (nBit < num_bits)
	{
		unsigned short bit_val = 0;
		cbDBitIn(board_num, FIRSTPORTA, nBit + 8, &bit_val);
		return bit_val > 0;
	}
	else
		throw runtime_error("[MCCin::GetBit] no such bit: " + to_string<int>(nBit));

}


#endif //!_NO_MCC_DIO
