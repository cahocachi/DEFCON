
#ifndef _included_worldobject_h
#define _included_worldobject_h

#include "lib/tosser/llist.h"
#include "lib/tosser/bounded_array.h"
#include "lib/math/vector2.h"
#include "lib/math/fixed.h"

class Image;
class WorldObjectState;
class ActionOrder;

class WorldObject;

// cached world object queries
class WorldObjectReference
{
    int m_uniqueId;          // the real unique ID
    mutable int m_index;     // the index in the array

public:
    WorldObjectReference() { m_uniqueId = -1; m_index = -1; }
    WorldObjectReference( int uniqueId ) { m_uniqueId = uniqueId; m_index = -1; }
    operator int() const { return m_uniqueId; }
    WorldObjectReference & operator = ( WorldObjectReference const & other ) { m_uniqueId = other.m_uniqueId; m_index = other.m_index; return *this; }
    WorldObjectReference & operator = ( WorldObject const * obj );
    WorldObjectReference & operator = ( int uniqueId ) { m_uniqueId = uniqueId; m_index = -1; return *this; }

    friend class World;
};

class WorldObject
{
public:
    enum
    {
        TypeInvalid,
        TypeCity,
        TypeSilo,
        TypeRadarStation,        
        TypeNuke,
        TypeExplosion,
        TypeSub,
        TypeBattleShip,
        TypeAirBase,
        TypeFighter,
        TypeBomber,
        TypeCarrier,
		TypeTornado,
        TypeSaucer,
        NumObjectTypes
    };

    enum
    {
        TargetTypeDefconRequired=-3,
        TargetTypeOutOfStock=-2,
        TargetTypeOutOfRange=-1,
        TargetTypeInvalid=0,
        TargetTypeValid,
        TargetTypeLaunchNuke,
        TargetTypeLaunchFighter,
        TargetTypeLaunchBomber,
        TargetTypeLand
    };

    int     m_type;
    int     m_teamId;
    WorldObjectReference m_objectId;
    Fixed   m_longitude;
    Fixed   m_latitude;
    int     m_life;                                     // Cities population, or 1/0 for sea units, or 0-30 for ground units
	int		m_lastHitByTeamId;
    bool    m_selectable;
    
    typedef Vector2<Fixed> Direction;
    Direction m_vel;

    LList   <WorldObjectState *> m_states;
    int     m_currentState;
    int     m_previousState;
    Fixed   m_stateTimer;                               // Time until we're in this state
    Fixed   m_previousRadarRange;                       // The size we previously added into the radar grid

    BoundedArray<bool>      m_visible;
    BoundedArray<bool>      m_seen;                     // seen objects memory
    BoundedArray<Vector2<Fixed> >   m_lastKnownPosition;
    BoundedArray<Direction>         m_lastKnownVelocity;
    BoundedArray<Fixed>     m_lastSeenTime;
    BoundedArray<int>       m_lastSeenState;
    
    LList<ActionOrder *> m_actionQueue;

    int     m_fleetId;
    int     m_nukeSupply;               // for reloading bombers
    bool    m_offensive;                // used by AI for tracking whether unit is offensive or defensive
    Fixed   m_aiTimer;
    Fixed   m_aiSpeed;
    Fixed   m_ghostFadeTime;
    WorldObjectReference m_targetObjectId;
    bool    m_isRetaliating;
    bool    m_forceAction;              // forces ai unit to act on a certain state regardless of team/fleet state

    int     m_numNukesInFlight;
    int     m_numNukesInQueue;
    double  m_nukeCountTimer;

    int     m_maxFighters;
    int     m_maxBombers;               // max number of aircraft this unit can hold


protected:
    char    bmpImageFilename[256];
    Fixed   m_radarRange;
    Fixed   m_retargetTimer;            // object is allowed to search for a new target when this = 0
    
public:
    WorldObject();
    virtual ~WorldObject();

    virtual void        InitialiseTimers();

    void                SetType         ( int type );
    void                SetTeamId       ( int teamId );
    void                SetPosition     ( Fixed longitude, Fixed latitude );
    
    void                SetRadarRange   ( Fixed radarRange );
    virtual Fixed       GetRadarRange   ();

    void                AddState        ( char *stateName, Fixed prepareTime, Fixed reloadTime, Fixed radarRange, 
                                          Fixed actionRange, bool isActionable, int numTimesPermitted = -1, int defconPermitted = 5 );
    
    virtual bool        CanSetState     ( int state );
    virtual void        SetState        ( int state );
    virtual bool        IsActionable    ();
    virtual Fixed       GetActionRange  ();
    virtual Fixed       GetActionRangeSqd();
    virtual void        Action          ( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude );
    virtual void        FleetAction     ( WorldObjectReference const & targetObjectId );
    virtual bool        IsHiddenFrom    ();

    virtual bool        Update          ();

    // information passed around on rendering so nothing has to be calculated twice
    struct RenderInfo
    {
        // filled by outside caller:
        float m_xOffset; // add this to all x coordinates to get the object into the viewport

        // filled by WorldObject/MovingObject:
        Vector2< float > m_position; // actual position with offset applied
        Vector2< float > m_velocity; // velocity as float vector
        Vector2< float > m_direction; // normalised velocity, only used for planes

        // filled by MovingObject::PrepareRender
        int m_maxHistoryRender[WorldObject::NumObjectTypes]; // maximal history size per object type
        float m_predictionTime; // time to extrapolate for

        inline void FillPosition( WorldObject * object );
    };

    static void         PrepareRender   ( RenderInfo & renderInfo );
    virtual void        Render          ( RenderInfo & renderInfo );
    
	virtual void		RunAI			();
    void                RenderCounter   ( int counter );
    virtual void        RenderGhost     ( int teamId, RenderInfo & renderInfo );

    virtual Fixed       GetSize         ();
    bool                CheckCurrentState();
    bool                IsMovingObject  ();

    virtual void        RequestAction   ( ActionOrder *_action );
    virtual bool        IsActionQueueable();
    void                ClearActionQueue();
    void                ClearLastAction();

    virtual void        FireGun         ( Fixed range );

    virtual bool        UsingNukes      ();
    virtual bool        UsingGuns       ();
    virtual void        NukeStrike      ();

    virtual void        Retaliate       ( WorldObjectReference const & attackerId );

    void                SetTargetObjectId ( WorldObjectReference const & targetObjectId );
    int                 GetTargetObjectId ();

    virtual bool        IsPinging();

    virtual int         GetAttackOdds   ( int _defenderType );

    static char         *GetName        ( int _type );
    static int          GetType         ( char *_name );
	static char         *GetTypeName    ( int _type );
	static WorldObject  *CreateObject   ( int _type );

    virtual int         IsValidCombatTarget     ( int _objectId );                                      // returns TargetType...
    virtual int         IsValidMovementTarget   ( Fixed longitude, Fixed latitude );                    //

    virtual bool        CanLaunchFighter();
    virtual bool        CanLaunchBomber ();
    bool                LaunchBomber    ( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude );
    bool                LaunchFighter   ( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude );
    virtual bool        SetWaypointOnAction();

    virtual void        CeaseFire       ( int teamId );
    virtual int         CountIncomingFighters();
    
    virtual char        *LogState();

    virtual Image       *GetBmpImage     ( int state );

    char                *GetBmpBlurFilename();
};



/*
 * ============================================================================
 * class WorldObjectState
 */

class WorldObjectState
{
public:
    char    *m_stateName;                   // eg "Launch ICBM"
    Fixed   m_timeToPrepare;                // ie time to get ready for launch
    Fixed   m_timeToReload;                 // ie minimum time between launches

    Fixed   m_radarRange;
    Fixed   m_actionRange;
    bool    m_isActionable;

    int     m_numTimesPermitted;            // Number of times a particular state can be used
    int     m_defconPermitted;              // The required defcon level before this state is usable

    ~WorldObjectState();

    char *GetStateName();                   // "Launch ICBM"
    char *GetPrepareName();                 // "Prepare to Launch ICBM"
    char *GetPreparingName();               // "Preparing to Launch ICBM"
    char *GetReadyName();                   // "Ready to Launch ICBM"
};



// ============================================================================

class ActionOrder
{
public:
    WorldObjectReference m_targetObjectId;
    Fixed   m_longitude;
    Fixed   m_latitude;
    bool    m_pursueTarget;   // tells the objects fleet to pursue target

public:
    ActionOrder()
        :   m_targetObjectId(-1),
            m_longitude(0),
            m_latitude(0),
            m_pursueTarget(false)
    {
    }
};

inline WorldObjectReference &  WorldObjectReference::operator = ( WorldObject const * obj )
{
    if( obj )
    {
        return operator = ( obj->m_objectId );
    }
    else
    {
        return operator = ( -1 );
    }
}

inline void WorldObject::RenderInfo::FillPosition( WorldObject * object )
{
    m_position.x = object->m_longitude.DoubleValue()+m_xOffset;
    m_position.y = object->m_latitude.DoubleValue();
    m_velocity.x = object->m_vel.x.DoubleValue();
    m_velocity.y = object->m_vel.y.DoubleValue();
    m_position += m_velocity * m_predictionTime;
}

#endif
