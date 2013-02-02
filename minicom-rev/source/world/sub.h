
#ifndef _included_sub_h
#define _included_sub_h

#include "world/naval/navalunit.h"


class Sub : public NavalUnit
{    
protected:
    bool    m_hidden;

    WorldObject *FindTarget      ();
    bool         IsSafeTarget    ( Fleet *_fleet );

public:

    Sub();

    void    Action          ( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude );
    bool    IsHiddenFrom    ();
    bool    Update          ();
    void    Render          ( RenderInfo & renderInfo );   
    void    RunAI           ();
    //int     GetAttackState  ();
    bool    IsIdle          ();
    bool    UsingGuns       ();
    bool    UsingNukes      ();
    void    SetState        ( int state );
    void    ChangePosition  ();
    bool    IsActionQueueable();
    bool    IsGroupable     ( int unitType );
    bool    IsPinging       ();
    void    FleetAction     ( WorldObjectReference const & targetObjectId );

    void    RequestAction   ( ActionOrder *_action );

    int     GetAttackOdds           ( int _defenderType );
    int     IsValidCombatTarget     ( int _objectId );
    int     IsValidMovementTarget   ( Fixed longitude, Fixed latitude );

	int		GetNukeCount	();
};


#endif
