
#ifndef _included_movingobject_h
#define _included_movingobject_h

#include "world/worldobject.h"

class Blip;

class MovingObject : public WorldObject
{
protected:
    Fixed   m_speed;     
    Fixed   m_turnRate;

    LList   <Vector2<float> > m_history;
    Fixed   m_historyTimer;
    int     m_maxHistorySize;

public:
    enum
    {
        MovementTypeLand,
        MovementTypeSea,
        MovementTypeAir
    };
    int     m_movementType;

    Fixed   m_targetLongitude;
    Fixed   m_targetLatitude;
    Fixed   m_range;

    Fixed   m_finalTargetLongitude;
    Fixed   m_finalTargetLatitude;
    int     m_targetNodeId;

    // Fixed   m_targetLongitudeAcrossSeam;
    // Fixed   m_targetLatitudeAcrossSeam;

    Fixed   m_pathCalcTimer;
    bool    m_blockHistory;
    int     m_isLanding;

public:
    MovingObject();
    ~MovingObject();

    void            InitialiseTimers();

    bool            Update();

    bool            IsValidPosition ( Fixed longitude, Fixed latitude );
    bool            MoveToWaypoint  ();                                   // returns true upon arrival
    void            Render          ();
    void            RenderGhost     ( int teamId );
    void            RenderHistory   ();

    virtual void    SetWaypoint     ( Fixed longitude, Fixed latitude );

    void            CalculateNewPosition( Fixed *newLongitude, Fixed *newLatitude );
    void            CrossSeam();

    virtual int     GetAttackState  ();
    virtual bool    IsIdle          ();

    virtual void    Land            ( int targetId );

    void            ClearWaypoints  ();
    void            AutoLand        ();
    int             GetClosestLandingPad();

    void            GetInterceptionPoint( WorldObject *target, Fixed *interceptLongitude, Fixed *interceptLatitude );

    virtual int     IsValidMovementTarget( Fixed longitude, Fixed latitude );

    virtual void    Retaliate       ( int attackerId );

    void            Ping            ();
    void            SetSpeed        ( Fixed speed );

    char            *LogState();

    virtual int     GetTarget( Fixed range );

private:
    bool            GetClosestLandingPad( BoundedArray<int> const & alreadyLanding, Fixed const & turnRadius, int & padId, int & nearestNonViableId );
};

/*
class Blip
{
public:

    enum
    {
        TargetNode,
        TargetEnd, 
        TargetSeam,
        TargetNodeAcrossSeam,
        TargetFinalAcrossSeam
    };
    float   m_longitude;
    float   m_latitude;
    float   m_targetLongitude;
    float   m_targetLatitude;
    Vector2 m_blipVel;
    int     m_targetNodeId;
    int     m_currentNodeId;
    float   m_nodeCheckTimer;
    int     m_targetType;

    LList   <Vector2 *> m_history;
    float   m_historyTimer;
};
*/

#endif
