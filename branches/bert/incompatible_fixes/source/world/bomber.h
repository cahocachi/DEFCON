
#ifndef _included_bomber_h
#define _included_bomber_h

#include "world/movingobject.h"


class Bomber : public MovingObject
{
public:
    Fixed   m_nukeTargetLongitude;
    Fixed   m_nukeTargetLatitude;

    bool    m_bombingRun;   // set to true if the bomber is launched to nuke a ground target, in which case it will land after firing
    
public:

    Bomber();

    void    Action          ( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude );
    Fixed   GetActionRange  ();
    bool    Update          ();
    void    Render          ( float xOffset );
    void    RunAI           ();
    bool    UsingNukes      ();
    void    SetNukeTarget   ( Fixed longitude, Fixed latitude );
    void    Retaliate       ( WorldObjectReference const &  attackerId );
    void    SetState        (int state );

    int     GetAttackOdds       ( int _defenderType );
    int     IsValidCombatTarget ( int _objectId );

    void    CeaseFire       ( int teamId );
    bool    SetWaypointOnAction();

};

#endif
