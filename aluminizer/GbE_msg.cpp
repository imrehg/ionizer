#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "messaging.h"

#include <assert.h>
#include <stdexcept>
#include <QTcpSocket>
#include <iostream>

QTcpSocket* pGbE_msg_TCP = 0;

#define my_htons(A) ((((unsigned short)(A) & 0xff00) >> 8) | \
                     (((unsigned short)(A) & 0x00ff) << 8))

#define my_htonl(A) ((((unsigned int)(A) & 0xff000000) >> 24) | \
                     (((unsigned int)(A) & 0x00ff0000) >> 8)  | \
                     (((unsigned int)(A) & 0x0000ff00) << 8)  | \
                     (((unsigned int)(A) & 0x000000ff) << 24))

#define my_ntohs  my_htons
#define my_ntohl  my_htonl

bool debugMsg = false;

using namespace std;

int GbE_msg::snd()
{
	assert(pGbE_msg_TCP);

	unsigned nRemaining = MIN_MSG_LENGTH;
	unsigned nSent = 0;
	unsigned nComm;

	if (debugMsg)
	{
		printf("[snd msg %08X] %08X (%u bytes)...", id, what, nRemaining);
		fflush(stdout);
	}

	id = my_htonl(id);
	what = my_htonl(what);

	char* p = (char*)(this);

	while (nRemaining)
	{
		nComm = pGbE_msg_TCP->write(p, nRemaining);
		p += nComm;
		nSent += nComm;
		nRemaining -= nComm;

		if (nRemaining > MIN_MSG_LENGTH)
			throw runtime_error("snd message too long");
	}

	if (debugMsg) printf(" done\r\n");

	return nSent;
}

int GbE_msg::rcv()
{
	assert(pGbE_msg_TCP);
	if (debugMsg)
	{
		printf("[rcv msg ");
		fflush(stdout);
	}

	char* p = (char*)(this);
	unsigned nRemaining = MIN_MSG_LENGTH;
	unsigned nRead = 0;
	unsigned nComm;

//	pGbE_msg_TCP->waitForReadyRead(-1);
	while (nRemaining)
	{
		nComm = pGbE_msg_TCP->read(p, nRemaining);
		p += nComm;
		nRead += nComm;
		nRemaining -= nComm;

		if (nRemaining > MIN_MSG_LENGTH)
			throw runtime_error("rcv message too long");

		if (nRemaining != 0)
			pGbE_msg_TCP->waitForReadyRead(-1);
	}

	id = my_ntohl(id);
	what = my_ntohl(what);

	if (debugMsg) printf("[%08X] %08X\r\n", id, what);

	return nRead;
}

void GbE_msg::insertU(unsigned loc, unsigned u)
{
	assert(loc < MSG_STD_PAYLOAD_SIZE);
	m[loc] = my_htonl(u);
}

void GbE_msg::insertS(unsigned loc, const char* s)
{
	unsigned length = strlen(s) + 1;

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
	return my_ntohl(m[loc]);
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

