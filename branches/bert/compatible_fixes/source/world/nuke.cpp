#include "lib/universal_include.h"
#include "lib/language_table.h"
#include "lib/sound/soundsystem.h"
#include "lib/math/math_utils.h"

#include <math.h>

#include "lib/resource/resource.h"
#include "lib/resource/image.h"
#include "lib/render/renderer.h"
#include "lib/math/vector3.h"
#include "lib/math/random_number.h"
#include "lib/profiler.h"

#include "app/app.h"
#include "app/globals.h"

#include "renderer/map_renderer.h"

#include "world/world.h"
#include "world/nuke.h"
#include "world/team.h"
#include "world/city.h"
#include "world/bomber.h"


Nuke::Nuke()
:   MovingObject(),
    m_prevDistanceToTarget( Fixed::MAX ),
    m_newLongitude(0),
    m_newLatitude(0),
    m_targetLocked(false)
{
    SetType( TypeNuke );

    strcpy( bmpImageFilename, "graphics/nuke.bmp" );

    m_radarRange = 0;
    m_speed = Fixed::Hundredths(20);
    m_selectable = true;
    m_maxHistorySize = -1;
    m_range = Fixed::MAX;
    m_turnRate = Fixed::Hundredths(1);
    m_movementType = MovementTypeAir;

    AddState( LANGUAGEPHRASE("state_ontarget"), 0, 0, 0, Fixed::MAX, false );
    AddState( LANGUAGEPHRASE("state_disarm"), 100, 0, 0, 0, false );
}


void Nuke::Action( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude )
{
    //m_newLongitude = longitude;
    //m_newLatitude = latitude;
}

void Nuke::SetWaypoint( Fixed longitude, Fixed latitude )
{
    if( m_targetLocked )
    {
        return;
    }
    MovingObject::SetWaypoint( longitude, latitude );

    Vector2<Fixed> target( m_targetLongitude, m_targetLatitude );
    Vector2<Fixed> pos( m_longitude, m_latitude );
    m_totalDistance = (target - pos).Mag();

    if( m_targetLongitude >= m_longitude )
    {
        m_curveDirection = 1;
    }
    else
    {
        m_curveDirection = -1;
    }
}


bool Nuke::Update()
{
    //
    // Are we disarmed?

    if( m_currentState == 1 && m_stateTimer <= 0 )
    {
        if( m_teamId == g_app->GetWorld()->m_myTeamId )
        {
            char message[64];
            sprintf( message, LANGUAGEPHRASE("message_disarmed") );
            g_app->GetWorld()->AddWorldMessage( m_longitude, m_latitude, m_teamId, message, WorldMessage::TypeObjectState );
        }
        return true;
    }


    //
    // Is our waypoint changing?

    Fixed timePerUpdate = SERVER_ADVANCE_PERIOD * g_app->GetWorld()->GetTimeScaleFactor();
    
    if( m_newLongitude != 0 || m_newLatitude != 0 )
    {
        Fixed factor1 = m_turnRate * timePerUpdate * 2;
        Fixed factor2 = 1 - factor1;
        m_targetLongitude = ( m_newLongitude * factor1 ) + ( m_targetLongitude * factor2 );
        m_targetLatitude = ( m_newLatitude * factor1 ) + ( m_targetLatitude * factor2 );

        if( ( m_targetLongitude - m_newLongitude ).abs() < 1 &&
			( m_targetLatitude - m_newLatitude ).abs() < 1 )
        {
            SetWaypoint( m_newLongitude, m_newLatitude );
            m_newLongitude = 0;
            m_newLatitude = 0;
        }
    }


    //
    // Move towards target

    Direction target( m_targetLongitude, m_targetLatitude );
    Direction pos( m_longitude, m_latitude );
    Fixed remainingDistance = (target - pos).Mag();
    Fixed fractionDistance = 1 - remainingDistance / m_totalDistance;

    Direction front = (target - pos).Normalise();  
    Fixed fractionNorth = 5 * m_latitude.abs() / ( 200 / 2 );
    fractionNorth = max( fractionNorth, 3 );
    
    front.RotateAroundZ( Fixed::PI / (fractionNorth * m_curveDirection) );
    front.RotateAroundZ( fractionDistance * (-m_curveDirection * Fixed::PI/fractionNorth) );

    if( pos.y > 85 )
    {
        // We are dangerously far north
        // Make sure we dont go off the top of the world
        Fixed extremeFractionNorth = (pos.y - 85) / 15;
        Clamp( extremeFractionNorth, Fixed(0), Fixed(1) );
        front.y *= ( 1 - extremeFractionNorth );
        front.Normalise();
    }

    m_vel = front * (m_speed/2 + m_speed/2 * fractionDistance * fractionDistance);

    Fixed newLongitude = m_longitude + m_vel.x * timePerUpdate;
    Fixed newLatitude = m_latitude + m_vel.y * timePerUpdate;
    Fixed newDistance = g_app->GetWorld()->GetDistance( newLongitude, newLatitude, m_targetLongitude, m_targetLatitude);

    if( newLongitude <= -180 ||
        newLongitude >= 180 )
    {
        m_longitude = newLongitude;
        CrossSeam();

        newLongitude = m_longitude;
        newDistance = g_app->GetWorld()->GetDistance( newLongitude, newLatitude, m_targetLongitude, m_targetLatitude );
    }

    if( newDistance < 2 &&
        newDistance >= remainingDistance )
    {
        m_targetLongitude = 0;
        m_targetLatitude = 0;
        m_vel.Zero();
        g_app->GetWorld()->CreateExplosion( m_teamId, m_longitude, m_latitude, 100 );
        g_soundSystem->TriggerEvent( SoundObjectId(m_objectId), "Detonate" );
        return true;
    }
    else
    {
        m_range -= ( m_vel * Fixed(timePerUpdate) ).Mag();
        if( m_range <= 0 )
        {
            m_life = 0;
			m_lastHitByTeamId = -1;
            g_app->GetWorld()->AddOutOfFueldMessage( m_objectId );
        }
        m_longitude = newLongitude;
        m_latitude = newLatitude;
    }

    return MovingObject::Update();
}

void Nuke::Render()
{
    MovingObject::Render();
}

void Nuke::FindTarget( int team, int targetTeam, int launchedBy, Fixed range, Fixed *longitude, Fixed *latitude )
{
    int objectId = -1;
    Nuke::FindTarget( team, targetTeam, launchedBy, range, longitude, latitude, &objectId );
}

void Nuke::FindTarget( int team, int targetTeam, int launchedBy, Fixed range, Fixed *longitude, Fixed *latitude, int *objectId )
{
    START_PROFILE("Nuke::FindTarget");
    
    World * world = g_app->GetWorld();

    WorldObject *launcher = world->GetWorldObject(launchedBy);
    if( !launcher ) 
    {
        END_PROFILE("Nuke::FindTarget");
        return;
    }

    LList<int> validTargets;
        
    for( int i = 0; i < world->m_objects.Size(); ++i )
    {
        if( world->m_objects.ValidIndex(i) )
        {
            WorldObject *obj = world->m_objects[i];
            if( obj->m_teamId == targetTeam &&
                obj->m_seen[team] &&
                !obj->IsMovingObject() )
            {
                Fixed distanceSqd = world->GetDistanceSqd( launcher->m_longitude, launcher->m_latitude, obj->m_longitude, obj->m_latitude);
                if( distanceSqd <= range * range )
                {                    
                    int numTargetedNukes = CountTargetedNukes( team, obj->m_longitude, obj->m_latitude );
                    
                    if( (obj->m_type == WorldObject::TypeRadarStation && numTargetedNukes < 2 ) ||
                        (obj->m_type != WorldObject::TypeRadarStation && numTargetedNukes < 4 ) )
                    {
                        validTargets.PutData(obj->m_objectId);
                    }
                }
            }
        }
    }

    if( validTargets.Size() > 0 )
    {
        int targetId = syncrand() % validTargets.Size();
        int objIndex = validTargets[ targetId ];
        WorldObject *obj = world->GetWorldObject(objIndex);
        if( obj )
        {
            *longitude = obj->m_longitude;
            *latitude = obj->m_latitude;
            *objectId = obj->m_objectId;
            END_PROFILE("Nuke::FindTarget");
            return;
        }
    }

    Team *friendlyTeam = world->GetTeam( team );

    int maxPop = 500000;        // Don't bother hitting cities with less than 0.5M survivors

    for( int i = 0; i < world->m_cities.Size(); ++i )
    {
        if( world->m_cities.ValidIndex(i) )
        {
            City *city = world->m_cities[i];

            if( !world->IsFriend( city->m_teamId, team) && 
				world->GetDistanceSqd( city->m_longitude, city->m_latitude, launcher->m_longitude, launcher->m_latitude) <= range * range)               
            {
                int numTargetedNukes = CountTargetedNukes(team, city->m_longitude, city->m_latitude);
                int estimatedPop = City::GetEstimatedPopulation( team, i, numTargetedNukes );
                if( estimatedPop > maxPop )
                {
                    maxPop = estimatedPop;
                    *longitude = city->m_longitude;
                    *latitude = city->m_latitude;
                    *objectId = -1;
                }
            }
        }
    }
    
    END_PROFILE("Nuke::FindTarget");
}

int Nuke::CountTargetedNukes( int teamId, Fixed longitude, Fixed latitude )
{
    World * world = g_app->GetWorld();
    int targetedNukes = 0;
    for( int i = 0; i < world->m_objects.Size(); ++i )
    {
        if( world->m_objects.ValidIndex(i) )
        {
            if( world->m_objects[i]->m_type == WorldObject::TypeNuke )
            {
                MovingObject *obj = (MovingObject *)world->m_objects[i];
                Fixed targetLongitude = obj->m_targetLongitude;
                if( targetLongitude > 180 )
                {
                    targetLongitude -= 360;
                }
                else if( targetLongitude < -180 )
                {
                    targetLongitude += 360;
                }

                if( obj->m_teamId == teamId &&
                    targetLongitude == longitude &&
                    obj->m_targetLatitude == latitude )
                {
                    ++targetedNukes;
                }
            }
            else if( world->m_objects[i]->m_type == WorldObject::TypeBomber )
            {
                Bomber *obj = (Bomber *)world->m_objects[i];
                if( obj->m_teamId == teamId &&
                    obj->m_nukeTargetLongitude == longitude &&
                    obj->m_nukeTargetLatitude == latitude )
                {
                    ++targetedNukes;
                }
            }
            else if( world->m_objects[i]->m_type == WorldObject::TypeSub ||
                     world->m_objects[i]->m_type == WorldObject::TypeSilo)
            {
                WorldObject *obj = world->m_objects[i];
                int nukeState = 0;
                if( obj->m_type == WorldObject::TypeSub ) 
                {
                    nukeState = 2;
                }
                if( obj->m_teamId == teamId &&
                    obj->m_currentState == nukeState )
                {
                    for( int j = 0; j < obj->m_actionQueue.Size(); ++j )
                    {
                        if( obj->m_actionQueue[j]->m_longitude == longitude &&
                            obj->m_actionQueue[j]->m_latitude == latitude )
                        {
                            targetedNukes++;
                        }
                    }
                }
            }
            else if( world->m_objects[i]->m_type == WorldObject::TypeAirBase ||
                     world->m_objects[i]->m_type == WorldObject::TypeCarrier )
            {
                WorldObject *obj = world->m_objects[i];
                if( obj->m_teamId == teamId &&
                    obj->m_currentState == 1 )
                {
                    for( int j = 0; j < obj->m_actionQueue.Size(); ++j )
                    {
                        if( obj->m_actionQueue[j]->m_longitude == longitude &&
                            obj->m_actionQueue[j]->m_latitude == latitude )
                        {
                            targetedNukes++;
                        }
                    }
                }
            }
        }
    }
    return targetedNukes;
}


void Nuke::CeaseFire( int teamId )
{
    if( g_app->GetMapRenderer()->IsValidTerritory( teamId, m_targetLongitude, m_targetLatitude, false ) )
    {
        SetState(1);
    }
}

int Nuke::IsValidMovementTarget( Fixed longitude, Fixed latitude )
{
    return TargetTypeInvalid;
}

void Nuke::LockTarget()
{
    m_targetLocked = true;
}
