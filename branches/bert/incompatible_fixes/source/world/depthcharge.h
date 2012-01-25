
#ifndef _included_depthcharge_h
#define _included_depthcharge_h

#include "world/movingobject.h"


class DepthCharge : public GunFire
{
public:
    Fixed   m_timer;

public:

    DepthCharge( Fixed range );

    bool    Update          ();
    void    Render          ( RenderInfo & renderInfo );
    void    Impact();
}; 


#endif
