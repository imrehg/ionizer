#ifdef HAS_HFS

#include "Transition.h"

namespace physics
{

class HFS_Al_II_SingletS0 : public HFS
{
public:
	HFS_Al_II_SingletS0();
};

class HFS_Al_II_TripletP0 : public HFS
{
public:
	HFS_Al_II_TripletP0();
};

class HFS_Al_II_TripletP1 : public HFS_BreitRabi
{
public:
	HFS_Al_II_TripletP1(double F);
};

} //namespace physics

#endif //HAS_HFS