#include <QString>

#include "RS232device.h"
#include <stdexcept>
#include <sstream>

#include <cstdio>
#include <iostream>


#ifdef WIN32
#include <windows.h>
std::string GetWindowsErrorMsg(unsigned error_code);
int read(HANDLE hCOM, char* rcv_buff, unsigned nToRead)
{
	DWORD nRead = 0;

	ReadFile(hCOM, rcv_buff, nToRead, &nRead, 0);
	return nRead;
}
#else
typedef int HANDLE;
#define INVALID_HANDLE_VALUE (-1)
#include <stdio.h>   /* Standard input/output definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#endif


using namespace std;

bool bDebugRS232 = false;

RS232device::RS232device(const std::string& port_name, unsigned baud) :
	port_name(port_name),
	baud(baud),
	hCOM(INVALID_HANDLE_VALUE)
{
	try
	{
		hCOM = OpenSerialPort(port_name, baud);
	}
	catch (runtime_error e)
	{
		cout << "ERROR: " << e.what() << endl;
	}
}


RS232device::~RS232device()
{
	if (isPortOpen())
	{
#ifdef WIN32
		CloseHandle(hCOM);
#else
		close(hCOM);
#endif
	}
}

bool RS232device::isPortOpen()
{
	return !(hCOM == INVALID_HANDLE_VALUE);
}


std::string RS232device::sendCmd_getResponse(const std::string& s, char line_end)
{
	sendCmd(s);
	return getResponse(line_end);
}

std::string RS232device::sendCmd_getResponseN(const std::string& s, unsigned nToRead)
{
	sendCmd(s);
	return getResponseN(nToRead);
}

void RS232device::sendCmd(const std::string& s)
{
	if ( !isPortOpen() )
		hCOM = OpenSerialPort(port_name, baud);

	if ( !isPortOpen() )
	{
		cerr << "unable to open serial port: " <<  port_name << endl;
		throw runtime_error("unable to open serial port: " + port_name);
	}

#ifdef WIN32
	if (s.length() > 0)
	{
		string s_out = s + "\n";
		DWORD nWrote = 0;

		WriteFile(hCOM, s_out.c_str(), s_out.length(), &nWrote, 0);
		if (bDebugRS232) printf("Sent (%d bytes): %s \r\n", nWrote, s_out.c_str());
	}
#else
	if (s.length() > 0)
	{
		string s_out = s + "\n";
		int nWrote = 0;

		nWrote = write(hCOM, s_out.c_str(), s_out.length());
		if (bDebugRS232) printf("[%s] Sent (%d bytes): %s", port_name.c_str(), nWrote, s_out.c_str());
	}
#endif
}

std::string RS232device::getResponse(char line_end)
{
	int iRcv = 0;

	while (1)
	{
		int nNew = read(hCOM, rcv_buff + iRcv, 1024);

		if (nNew > 0)
		{
			iRcv += nNew;
			rcv_buff[iRcv] = 0;
		}

		if (iRcv)
			if (rcv_buff[iRcv - 1] == line_end)
			{
				if (bDebugRS232) printf("[%s] Recv (%d bytes): %s", port_name.c_str(), iRcv, rcv_buff);
				iRcv = 0;
				return string(rcv_buff);
			}
	}

	printf("WARNING: failed to receive line-end character: %d", (int)(line_end));
	return "";
}

std::string RS232device::getResponseN(unsigned nToRead)
{
	unsigned nNew = 0;
	int iRcv = 0;

	while (nNew < nToRead)
	{
		unsigned nRead = read(hCOM, rcv_buff + iRcv, 1024);
		nNew += nRead;
		iRcv += nNew;

		rcv_buff[iRcv] = 0;
	}

	return string(rcv_buff);
}

#ifdef WIN32
string GetWindowsErrorMsg(unsigned error_code)
{
	string msg = "";

	HMODULE hModule = NULL;  // default to system source
	LPSTR MessageBuffer;
	DWORD dwBufferLength;

	DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
	                      FORMAT_MESSAGE_IGNORE_INSERTS |
	                      FORMAT_MESSAGE_FROM_SYSTEM ;

	if (dwBufferLength = FormatMessageA(
	       dwFormatFlags,
	       hModule,                                                // module to get message from (NULL == system)
	       error_code,
	       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),              // default language
	       (LPSTR)&MessageBuffer,
	       0,
	       NULL
	       )
	    )
	{
		msg = MessageBuffer;
		LocalFree(MessageBuffer);
	}
	else
	{
		ostringstream oss;
		oss << "Windows error code #" << error_code;
		msg = oss.str();
	}


	return msg;
}


HANDLE RS232device::OpenSerialPort(const std::string& port_name, unsigned baud_rate)
{
	HANDLE hSerial = INVALID_HANDLE_VALUE;

	QString port_nameQS;

	port_nameQS = QString(port_name.c_str());
	wstring name = port_nameQS.toStdWString();

	//open the port
	hSerial = CreateFile(name.c_str(), GENERIC_READ | GENERIC_WRITE,
	                     0,   0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hSerial == INVALID_HANDLE_VALUE)
		throw runtime_error( GetWindowsErrorMsg( GetLastError() ) );

	//configure the port
	DCB PortDCB;

	// Initialize the DCBlength member.
	PortDCB.DCBlength = sizeof(DCB);

	// Get the default port setting information.
	GetCommState(hSerial, &PortDCB);

	// Change the DCB structure settings.
	PortDCB.BaudRate = baud_rate;          // Current baud
	PortDCB.fBinary = TRUE;                // Binary mode; no EOF check
	PortDCB.fParity = TRUE;                // Enable parity checking
	PortDCB.fOutxCtsFlow = FALSE;          // No CTS output flow control
	PortDCB.fOutxDsrFlow = FALSE;          // No DSR output flow control
	PortDCB.fDtrControl = DTR_CONTROL_ENABLE;
	// DTR flow control type
	PortDCB.fDsrSensitivity = FALSE;       // DSR sensitivity
	PortDCB.fTXContinueOnXoff = TRUE;      // XOFF continues Tx
	PortDCB.fOutX = FALSE;                 // No XON/XOFF out flow control
	PortDCB.fInX = FALSE;                  // No XON/XOFF in flow control
	PortDCB.fErrorChar = FALSE;            // Disable error replacement
	PortDCB.fNull = FALSE;                 // Disable null stripping
	PortDCB.fRtsControl = RTS_CONTROL_ENABLE;
	// RTS flow control
	PortDCB.fAbortOnError = FALSE;         // Do not abort reads/writes on
	// error
	PortDCB.ByteSize = 8;                  // Number of bits/byte, 4-8
	PortDCB.Parity = NOPARITY;             // 0-4=no,odd,even,mark,space
	PortDCB.StopBits = ONESTOPBIT;         // 0,1,2 = 1, 1.5, 2

	// Configure the port according to the specifications of the DCB
	// structure.
	if (!SetCommState(hSerial, &PortDCB))
		throw runtime_error( GetWindowsErrorMsg( GetLastError() ) );

	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = 1;
	timeouts.ReadTotalTimeoutMultiplier = 1;
	timeouts.ReadTotalTimeoutConstant = 10;
	timeouts.WriteTotalTimeoutMultiplier = 1;
	timeouts.WriteTotalTimeoutConstant = 10;

	SetCommTimeouts(hSerial, &timeouts);

	return hSerial;
}
#else
HANDLE RS232device::OpenSerialPort(const std::string& port_name, unsigned baud_rate)
{
	// See
	// Serial Programming Guide for POSIX Operating Systems

	int fd = open(port_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

	fcntl(fd, F_SETFL, FNDELAY);

	struct termios options;

	/*
	 * Get the current options for the port...
	 */

	tcgetattr(fd, &options);

	unsigned baud = 0;

	switch (baud_rate)
	{
	case  57600: baud = B57600; break;
	case 921600: baud = B921600; break;
	default: throw runtime_error("unsupported baud_rate");
	}

	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);

	/*
	 * Enable the receiver and set local mode...
	 */

	options.c_cflag |= (CLOCAL | CREAD);

	/*
	 * Set the new options for the port...
	 */

	tcsetattr(fd, TCSANOW, &options);

	return fd;
}
#endif //WIN32
