#ifndef INFO_INTERFACE_H
#define INFO_INTERFACE_H

#include <vector>
#include <string>

#include "remote_params.h"

class info_interface;
typedef std::vector<info_interface*> list_t;
extern list_t global_exp_list;
typedef std::vector<remote_param*> params_t;


class result_channel
{
public:
result_channel(std::vector<result_channel*>& v, const std::string& name) : name(name), result(0)
{
	v.push_back(this);
}

//if a name has an (s) at the end it gets a thick plot curve
//if a name has an (h) at the end it's not plotted
std::string name;
double result;
};


//! Interface to exchange info w/ PC and present a GUI
class info_interface
{
public:
info_interface(list_t* exp_list, const std::string& name) :
	name(name),
	page_id(0)
{
	if (exp_list)
	{
		exp_list->push_back(this);
		page_id = exp_list->size() - 1;
	}
}

virtual ~info_interface()
{
}

virtual void update_param_binary(unsigned param_id, unsigned length, const char* bin);

//! called after parameters have been updated
virtual void updateParams()
{
}

unsigned getNumRemoteActions() const
{
	return remote_actions.size();
}
const std::string& getRemoteActionName(unsigned i)
{
	return remote_actions.at(i);
}

unsigned remote_action(unsigned i)
{
	return remote_action(remote_actions.at(i).c_str());
}
virtual unsigned remote_action(const char*)
{
	return 0;
}

unsigned getID() const
{
	return page_id;
}
const std::string& getName() const
{
	return name;
}
unsigned getNumParams() const
{
	return params.size();
}

//! copy parameter definition into s
void defineParam(unsigned i, char* s);

//! update the parameter after the GUI changed.  Derived classes must call the base clase function.
virtual void updateParam(const char* name, const char* value);

//! copy parameter explanation (e.g. tool-tip) into s
void explainParam(unsigned i, char* s, unsigned len);

//! copy parameter value string into s
void get_param_val_str(unsigned i, char* s, unsigned len);

unsigned getNumChannels()
{
	return channels.size();
}
const std::string& getChannelName(unsigned i)
{
	return channels.at(i)->name;
}

//! Return data for plots on a GUI page.  The format is msg_out.m[0] = num_points, msg_out.m[1] = data[0], ...
virtual void getPlotData(unsigned iPlot, unsigned iStart, GbE_msg& msg_out);
virtual unsigned getNumPlots();

virtual void setCoefficients(const GbE_msg&, GbE_msg&)
{
}

public:
std::string name;

protected:
unsigned page_id;
std::vector<std::string> remote_actions;    //list of actions available from the remote (PC) interface
params_t params;
std::vector<result_channel*> channels;
};

//! Provide some info about the FPGA.  Allow debug messages to be enabled/disabled.
class infoFPGA : public info_interface
{
public:
infoFPGA(list_t* exp_list, const std::string& name);
virtual ~infoFPGA()
{
}

virtual unsigned remote_action(const char* s);
virtual void updateParams();

protected:
unsigned testAllocatedMemory(char* p, unsigned size);
void test_memory();

public:
rp_bool debug_params;
rp_bool debug_clock;
rp_bool debug_spi;
rp_string revision_info;
};

extern infoFPGA* iFPGA;

#endif // INFO_INTERFACE_H
