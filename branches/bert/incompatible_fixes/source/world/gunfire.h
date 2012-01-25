
#ifndef _included_gunfire_h
#define _included_gunfire_h

#include "world/movingobject.h"


class GunFire : public MovingObject
{
public:
    WorldObjectReference m_origin;
    bool    m_killShot;
    Fixed   m_attackOdds;
    Fixed   m_distanceToTarget;

public:

    GunFire( Fixed range );

    virtual bool    Update          ();
    virtual void    Render          ( RenderInfo & renderInfo );
    virtual void    Impact          ();
    bool            MoveToWaypoint  ();
    void            CalculateNewPosition( Fixed *newLongitude,Fixed *newLatitude, Fixed *newDistance );
}; 


#endif
