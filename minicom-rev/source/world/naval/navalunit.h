
#ifndef _included_navalunit_h
#define _included_navalunit_h

#include "world/worldobject.h"
#include "world/movingobject.h"


class NavalUnit : public MovingObject
{
public:
    NavalUnit();

    bool			IsValidPosition ( Fixed longitude, Fixed latitude );
};

#endif
