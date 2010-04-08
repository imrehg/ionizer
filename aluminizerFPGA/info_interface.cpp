#include "common.h"
#include "info_interface.h"
#include "remote_params.h"

#include <iostream>
using namespace std;

bool g_debug_spi = false;

//! todo - add length checking here
void info_interface::defineParam(unsigned i, char* s)
{
/*   cout << "[" << name << "::defineParam]" << " i == " << i << "  name == " << params.at(i)->getName() << endl;

   char buff[1024];
   params.at(i)->defineAsString(buff);
   cout << buff << "(l=" << strlen(buff) << ")" << endl;

   s[0] = 0;

   if(strlen(buff) <= 400)
 */
	params.at(i)->defineAsString(s);
}

//! copy paremater explanation (e.g. tool-tip) into s
void info_interface::explainParam(unsigned i, char* s, unsigned len)
{
	params.at(i)->explain(s, len);
}

//! copy parameter value string into s
void info_interface::get_param_val_str(unsigned i, char* s, unsigned len)
{
	params.at(i)->getValueString(s, len);
}

void info_interface::updateParam(const char* p_name, const char* value)
{
	for (size_t i = 0; i < params.size(); i++)
	{
		if (strcmp(params[i]->getName().c_str(), p_name) == 0)
		{
			if ( !params[i]->hasFlag(RP_FLAG_FPGA_UPDATES) )
				params[i]->setValueFromString(value);

//         char buff[256];
//         params[i]->getValueString(buff, 256);
//	      cout << "[" << name << "] " << p_name << " = " << buff << endl;
		}
	}
}

void info_interface::update_param_binary(unsigned param_id, unsigned length, const char* bin)
{
	params.at(param_id)->update_binary(length, bin);
}

unsigned info_interface::getNumPlots()
{
	return 0;
}

void info_interface::getPlotData(unsigned, unsigned, GbE_msg& msg_out)
{
	msg_out.insertU(0, 0);
}

#ifndef VERSION_STR
#define VERSION_STR "sim"
#endif

infoFPGA::infoFPGA(list_t* exp_list, const std::string& name) :
	info_interface(exp_list, name),
	debug_params("Debug params", &params, "value=0"),
	debug_clock("Debug clock state", &params, "value=0"),
	debug_spi("Debug spi", &params, "value=0"),
	revision_info("Revision", &params, "value=0")
{
	debug_params.set(false);
	debug_params.setFlag(RP_FLAG_UPDATE_IMMEDIATE);
	debug_clock.setFlag(RP_FLAG_UPDATE_IMMEDIATE);

	char buff[128];
	snprintf(buff, 128, "%s %s (%s)", __DATE__, __TIME__, VERSION_STR);

	revision_info.setFlag(RP_FLAG_FPGA_UPDATES | RP_FLAG_READ_ONLY | RP_FLAG_NOPACK);
	revision_info.set(string(buff));

	printf("Revision: %s\r\n", buff);

	remote_actions.push_back("TEST MEMORY");
}


unsigned infoFPGA::remote_action(const char* s)
{
	if (strcmp(s, "TEST MEMORY") == 0)
		test_memory();

	return 0;
}

void infoFPGA::updateParams()
{
	info_interface::updateParams();

	g_debug_spi = debug_spi;
}

unsigned infoFPGA::testAllocatedMemory(char* p, unsigned size)
{
	unsigned nErrors = 0;
	char test = rand();

	printf("testing 0x%02X ... ", (unsigned)test);
	for (unsigned i = 0; i < size; i++)
		p[i] = test;

	for (unsigned i = 0; i < size; i++)
	{
		if (p[i] != test)
		{
			nErrors++;

			printf("Error at memory location %p !  Wrote 0x%02X  Read 0x%02X\n", (p + i), (unsigned)test, (unsigned)(p[i]));
		}
	}

	if (nErrors == 0)
		printf("OK\n");

	return nErrors;
}

void infoFPGA::test_memory()
{
	bool useNewDelete = false;
	unsigned nErrors = 0;

	for (unsigned i = 9; i < 30; i++)
	{
		for (unsigned j = 0; j < 100; j++)
		{
			unsigned nAlloc = (1 << i);
			unsigned charSize = sizeof(char);

			printf("allocating %u x %u bytes ... ", nAlloc, charSize);
			fflush(stdout);

			try
			{
				char* p = 0;

				if (useNewDelete)
					p = new char[nAlloc];
				else
					p = (char*)malloc(nAlloc);

				if (p != 0)
				{
					printf(" address = %p  success\n", p);

					nErrors += testAllocatedMemory(p, nAlloc);

					printf("freeing %p \n", p);

					if (useNewDelete)
						delete [] p;
					else
						free(p);
				}
				else
				{
					nErrors++;
					printf("failure\n");
					break;
				}
			}
			catch (std::bad_alloc e)
			{
				nErrors++;
				printf("failure\n");
				break;
			}

			fflush(stdout);
		}
	}

	printf("test_memory result: %u errors\n", nErrors);
}
