
#ifndef _included_airunit_h
#define _included_airunit_h

#include "world/worldobject.h"
#include "world/movingobject.h"


class AirUnit : public MovingObject
{
public:
    AirUnit();

    bool			IsValidPosition ( Fixed longitude, Fixed latitude );

	virtual void    Render          (RenderInfo & renderInfo);

	virtual bool	CanLandOn			(int _type);

	virtual bool	SetWaypointOnAction();
};

#endif
