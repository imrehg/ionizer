#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "lwip/sockets.h"

#include "shared/src/messaging.h"

int GbE_socket = 0;

bool debugMsg = false;

int GbE_msg::snd()
{
	if (debugMsg) printf("[snd msg %08X] %08X ...", id, what);

	char* p = (char*)(this);
	int nRemaining = MIN_MSG_LENGTH;
	int nSent = 0;
	int nComm;

	while (nRemaining > 0)
	{
		int nComm = write(GbE_socket, p, nRemaining);
		nRemaining -= nComm;
		nSent += nComm;
		p += nComm;
	}


	if (debugMsg) printf(" done\r\n");

	return nSent;
}

int GbE_msg::rcv()
{
	if (debugMsg) printf("[rcv msg ");

	char* p = (char*)(this);
	int nRemaining = MIN_MSG_LENGTH;
	int nRead = 0;
	int nComm;

	while (nRemaining > 0)
	{
		int nComm = read(GbE_socket, p, nRemaining);
		nRemaining -= nComm;
		nRead += nComm;
		p += nComm;
	}

	if (debugMsg) printf("%08X] %08X\r\n", id, what);

	return nRead;
}

void GbE_msg::insertU(unsigned loc, unsigned u)
{
	assert(loc < MSG_STD_PAYLOAD_SIZE);
	m[loc] = u;
}

void GbE_msg::insertS(unsigned loc, const char* s)
{
	unsigned length = strlen(s);

	assert((loc + length / sizeof(unsigned)) < MSG_STD_PAYLOAD_SIZE);

	strcpy((char*)(m + loc), s);
}

void GbE_msg::insert2S(unsigned loc, const char* s1, const char* s2)
{
	unsigned length = strlen(s1) + strlen(s2) + 2;

	assert((loc + length / sizeof(unsigned)) < MSG_STD_PAYLOAD_SIZE);

	strcpy((char*)(m + loc), s1);
	strcpy((char*)(m + loc) + strlen(s1) + 1, s2);
}

unsigned GbE_msg::extractU(unsigned loc) const
{
	assert(loc < MSG_STD_PAYLOAD_SIZE);
	return m[loc];
}


char* GbE_msg::extractS(unsigned loc)
{
	assert(loc <= MSG_STD_PAYLOAD_SIZE);
	return (char*)(m + loc);
}

const char* GbE_msg::extractSC(unsigned loc) const
{
	assert(loc <= MSG_STD_PAYLOAD_SIZE);
	return (const char*)(m + loc);
}
