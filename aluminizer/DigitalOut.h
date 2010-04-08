#pragma once

class DigitalOut
{
public:
virtual ~DigitalOut()
{
}

virtual unsigned GetBit(unsigned nBit) = 0;
virtual void SetBit(unsigned nBit, unsigned value) = 0;
virtual unsigned NumStates() = 0;
};

class DigitalIn
{
public:
virtual ~DigitalIn()
{
}

virtual bool GetBit(unsigned nBit) = 0;
};

#ifndef _NO_MCC_DIO

class MCCout : public DigitalOut
{
public:
MCCout(int board_num, int port_num = 10, int port_type = 10);
virtual ~MCCout()
{
}

virtual unsigned GetBit(unsigned nBit);
virtual void SetBit(unsigned nBit, unsigned value);

virtual unsigned NumStates()
{
	return 2;
}

protected:
int board_num;
int port_num;
int port_type;

const unsigned num_bits;

std::vector<unsigned int> state;
};

class MCCin : public DigitalIn
{
public:
MCCin(int board_num, int port_num = 11, int port_type = 11);
virtual ~MCCin()
{
}

virtual bool GetBit(unsigned nBit);

protected:
int board_num;
int port_num;
int port_type;

const unsigned num_bits;
};

#else //!_NO_MCC_DIO

class SimDigitalOut : public DigitalOut
{
public:
SimDigitalOut()
{
}
virtual ~SimDigitalOut()
{
}

virtual unsigned GetBit(unsigned)
{
	return 0;
}
virtual void SetBit(unsigned, unsigned)
{
}

virtual unsigned NumStates()
{
	return 2;
}
};

class SimDigitalIn : public DigitalIn
{
public:
SimDigitalIn()
{
}
virtual bool GetBit(unsigned)
{
	return false;
}
};

class MCCout : public SimDigitalOut
{
public:
MCCout(int, int = 10, int = 10) : SimDigitalOut()
{
}
};

class MCCin : public SimDigitalIn
{
public:
MCCin(int, int = 11, int = 11) : SimDigitalIn()
{
}
};

#endif //!_NO_MCC_DIO

