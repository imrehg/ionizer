#pragma once

#include "experiments.h"

class xition_id
{
	char mFx2, pol, sb, power;
};

class xition_info
{
	unsigned fOn;
	unsigned t;
};

class xition_info : public interface_info
{
public:
	xition_info(list_t* exp_list, const std::string& name, float Fg, float Fe, int nSB) :
	  info_interface(exp_list, name)
    {
		AddParams();
    }

	virtual ~xition_info() 
	{
		//TODO: delete params
	}

	unsigned id2index(const xition_id& xid)
	{
	}


	void AddParams()
	{
		double mFg = -1*Fg;
		double mFe = -1*Fe;

		for(float mFg = -1*Fg; mFg<=Fg; mFg++)
			for(float mFe = -1*Fe; mFe<=Fe; mFe++)
			{
				if(IsAllowed(mFg, mFe))
					for(int sb=-1*nSB; sb<=nSB; sb++
					{
						pulses.push_back(new_pulse(mFg, mFe, sb));
					}
			}
	}

	dds_params* new_pulse(float mFg, float mFe, int sb)
	{
		string name = pulse_name(mFg, mFe, sb);

		return 

	bool IsAllowed(float mFg, float mFe)
	{
		return (fabs(mFg) <= Fg) && (fabs(mFe) <= Fe)
	}

	vector<dds_params*> pulses;
};

