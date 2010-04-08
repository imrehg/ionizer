#include "ExpSCAN.h"

class RunOvens : public ExperimentPage
{
public:
RunOvens(const string& sPageName, ExperimentsSheet* pSheet);
virtual ~RunOvens();

virtual void AddAvailableActions(std::vector<std::string>* v);
virtual void on_action(const std::string& s);

virtual void InitExperimentStart()
{
}
virtual void InitExperimentSwitch()
{
}

virtual run_status Run();

virtual void DefaultExperimentState()
{
}
virtual void FinishRun()
{
}

//turn Mg and Al ovens on/off
void setMgOven(bool b);
void setAlOven(bool b);

//turn on/off photo-ionization light
void setAlPI(bool b);

protected:

void set_output(int word);
void init_dio();

GUI_int BoardNumber;
GUI_int LoadTime;
GUI_int egun_delay_Mg;
GUI_int egun_delay_Al;
GUI_int shutter_Mg;
GUI_int shutter_Al;
GUI_double ElapsedTime;
GUI_progress Progress;
GUI_string Running;

int current_output_word;
vector<int> output_word;
vector<int> output_time;
unsigned shutter_ttl;
};
