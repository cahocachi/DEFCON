
#ifndef _included_tornado_h
#define _included_tornado_h

#include "world/movingobject.h"


class Tornado : public MovingObject
{
 
protected:
    
    int     m_targetObjectId;

	Fixed   m_totalDistance;
    Fixed   m_curveDirection;
    Fixed   m_prevDistanceToTarget;
    
    Fixed   m_newLongitude;             // Used to slowly turn towards new target
    Fixed   m_newLatitude;

	float	m_angle;

	Fixed	m_size;
    
    LList< int > m_deflectedNukeIds;
    
public:
    Tornado();
    
    void    Action          ( WorldObjectReference const & targetObjectId, float longitude, float latitude );
    Fixed   GetActionRange  ();
    bool    Update          ();
    void    Render          ( RenderInfo & renderInfo );

	Fixed	GetSize();
	void	SetSize( Fixed size );

    void    GetNewTarget    ();

};


#endif
