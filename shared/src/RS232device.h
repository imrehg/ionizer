#pragma once

#ifdef WIN32
typedef void* com_t;
#else
typedef int com_t;
#endif



class RS232device
{
public:
RS232device(const std::string& port_name, unsigned baud);
virtual ~RS232device();

bool isPortOpen();

//! send command via RS232 port, return response
void sendCmd(const std::string& s);
std::string sendCmd_getResponse(const std::string& s, char line_end = '\n');
std::string sendCmd_getResponseN(const std::string& s, unsigned nToRead);

std::string getResponse(char line_end = '\n');
std::string getResponseN(unsigned nToRead);

protected:
com_t OpenSerialPort(const std::string& port_name, unsigned baud_rate);

std::string port_name;
unsigned baud;

com_t hCOM;
char rcv_buff[1024];
bool bDebugRS232;
};
