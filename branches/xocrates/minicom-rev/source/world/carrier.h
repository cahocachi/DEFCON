
#ifndef _included_carrier_h
#define _included_carrier_h

#include "world/naval/navalunit.h"


class Carrier : public NavalUnit
{
protected:
    char bmpFighterMarkerFilename[256];
    char bmpBomberMarkerFilename[256];

public:

    Carrier();

    void    RequestAction       (ActionOrder *_action);
    void    Action              ( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude );
    bool    Update              ();
    void    Render              ( RenderInfo & renderInfo );
    void    RunAI               ();
    bool    IsActionQueueable   ();
    int     FindTarget          ();
    //int     GetAttackState      ();

    void    Retaliate           ( WorldObjectReference const & attackerId );
    bool    UsingNukes          ();
    void    FireGun             ( Fixed range );

    void    FleetAction     ( WorldObjectReference const & targetObjectId );
	bool    LaunchBomber    ( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude );
    bool    LaunchFighter   ( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude );

    int     GetAttackOdds   ( int _defenderType );
    
    bool    CanLaunchFighter();
    bool    CanLaunchBomber();

    //int     CountIncomingFighters();

    void    LaunchScout();

    int  IsValidCombatTarget     ( int _objectId );                                      // returns TargetType...
    int  IsValidMovementTarget   ( Fixed longitude, Fixed latitude );                    //

	int GetNukeCount();
};



#endif
