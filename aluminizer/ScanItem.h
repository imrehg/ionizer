class ScanItem
{
public:
ScanItem(const std::string& name) : name(name)
{
};
ScanItem(const std::string& name, int i) name(name + to_string<int>(i))
{
};

virtual void UpdateScanItem(double vNew, double v0) = 0;

const std::string name;
double ref;    //reference value from which the scan steps off.
};

class FScanItem
{
public:
FScanItem(const std::string& name) : ScanItem(name)
{
};
FScanItem(const std::string& name, int i) : ScanItem(name, i)
{
};

virtual void UpdateScanItem(double vNew, double v0);

const std::string name;
double ref;    //reference value from which the scan steps off.
};

class TScanItem
{
public:
TScanItem(const std::string& name)  : ScanItem(name)
{
};
TScanItem(const std::string& name, int i) : ScanItem(name, i)
{
};

virtual void UpdateScanItem(double vNew, double v0) = 0;

const std::string name;
double ref;    //reference value from which the scan steps off.
};

