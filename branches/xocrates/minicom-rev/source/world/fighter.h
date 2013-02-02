
#ifndef _included_fighter_h
#define _included_fighter_h

#include "world/air/airunit.h"


class Fighter : public AirUnit
{  
public:
    bool    m_playerSetWaypoint;

public:
    Fighter();

    void    Action          ( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude );
    bool    Update          ();
    // void    Render          ( RenderInfo & renderInfo );

	void	RunAI			();

    //int     GetAttackState  ();

    bool    UsingGuns       ();
    int     CountTargettedFighters( int targetId );

    int     IsValidCombatTarget( int _objectId );                                      // returns TargetType...

    void    Retaliate       ( WorldObjectReference const & attackerId );

    //bool    SetWaypointOnAction();
};


#endif
