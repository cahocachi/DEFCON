#include "lib/universal_include.h"
#include "lib/resource/resource.h"
#include "lib/resource/image.h"
#include "lib/render/renderer.h"
#include "lib/math/vector3.h"
#include "lib/math/random_number.h"
#include "lib/language_table.h"

#include "app/app.h"
#include "app/globals.h"

#include "renderer/map_renderer.h"

#include "interface/interface.h"

#include "world/world.h"
#include "world/fighter.h"
#include "world/team.h"
#include "world/city.h"
#include "world/worldoption.h"

static MovingUnitSettings s_fighterSettings( WorldObject::TypeFighter, 1, 10, 4, 45 );
static StateSettings s_fighterAttack( WorldObject::TypeFighter, "", 60, 20, 5, 10, true );

Fighter::Fighter()
:   MovingObject()
{
    Setup( TypeFighter, s_fighterSettings );

    strcpy( bmpImageFilename, "graphics/fighter.bmp" );

    // m_radarRange = 5;
    m_selectable = true;
    m_maxHistorySize = 10;

    m_movementType = MovementTypeAir;

    AddState( LANGUAGEPHRASE("state_attack"), s_fighterAttack );

    InitialiseTimers();
}

void Fighter::Action( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude )
{   
    m_targetObjectId = -1;
    World * world = g_app->GetWorld();

    if( m_currentState == 0 )
    {
        //SetWaypoint( longitude, latitude );
        WorldObject *target = world->GetWorldObject( targetObjectId );
        if( target )
        {
            if( target->m_visible[m_teamId] &&
                world->GetAttackOdds( m_type, target->m_type ) > 0)
            {
                m_targetObjectId = targetObjectId;
            }
        }
    }

    if( m_teamId == world->m_myTeamId &&
        targetObjectId == -1 )
    {
        g_app->GetMapRenderer()->CreateAnimation( MapRenderer::AnimationTypeActionMarker, m_objectId,
												  longitude.DoubleValue(), latitude.DoubleValue() );
    }

    MovingObject::Action( targetObjectId, longitude, latitude );
}

static WorldOption< int > s_fighterAlwaysAutoTarget( "FighterAlwaysAutoTarget", 1 );

bool Fighter::Update()
{   
    bool arrived = MoveToWaypoint();
    bool holdingPattern = false;

    if( m_targetObjectId != -1 )
    {
        WorldObject *targetObject = g_app->GetWorld()->GetWorldObject(m_targetObjectId);  
        if( targetObject )
        {
            if( targetObject->m_teamId == m_teamId &&
                ( targetObject->m_type == WorldObject::TypeCarrier ||
                  targetObject->m_type == WorldObject::TypeAirBase ) )
            {
                Land( targetObject );
                m_targetObjectId = -1;
            }
            else
            {
                if( m_targetLongitude == 0 && m_targetLatitude == 0 )
                {
                    SetWaypoint( targetObject->m_lastKnownPosition[m_teamId].x, 
                                 targetObject->m_lastKnownPosition[m_teamId].y );
                }
           
                if( arrived )
                {
                    if( !targetObject->m_visible[m_teamId] )
                    {
                        m_targetObjectId = -1;
                    }
                    holdingPattern = true;
                }

                if( m_stateTimer <= 0 )
                {
                    if( targetObject->m_visible[m_teamId] )
                    {
                        Fixed distanceSqd = g_app->GetWorld()->GetDistanceSqd( m_longitude, m_latitude, targetObject->m_longitude, targetObject->m_latitude);

                        if( distanceSqd <= GetActionRangeSqd() )
                        {
                            FireGun( GetActionRange() );
                        }
                    }
                    else
                    {
                        m_targetObjectId = -1;
                    }
                }
            }
        }
        else
        {
            m_targetObjectId = -1;
        }
    }

    if( m_targetLongitude == 0 && m_targetLatitude == 0 )
    {
        if( m_targetObjectId == -1 )
        {
            if( m_range <= 0 )
            {
                m_life = 0;
				m_lastHitByTeamId = m_teamId;
                g_app->GetWorld()->AddOutOfFueldMessage( m_objectId );
            }
        }
        else
        {
            holdingPattern = true;
        }
    }

    if( ( s_fighterAlwaysAutoTarget ? ( m_targetObjectId == -1 ) : IsIdle() ) && 
        m_retargetTimer <= 0 )
    {
        m_retargetTimer = 10;
        WorldObject *obj = g_app->GetWorld()->GetWorldObject( GetTarget( ( m_range < 40 ) ? m_range : 40 ) );
        if( obj )
        {
            // follow object if there are no other orders
            if( IsIdle() )
            {
                SetWaypoint( obj->m_longitude, obj->m_latitude );
            }
            m_targetObjectId = obj->m_objectId;
        }
        else if( IsIdle() )
        {
            Land( GetClosestLandingPad() );
        }
    }

    if( holdingPattern )
    {
        Fixed timePerUpdate = SERVER_ADVANCE_PERIOD * g_app->GetWorld()->GetTimeScaleFactor();

        m_vel.RotateAroundZ( Fixed::Hundredths(2) * timePerUpdate );

        if( m_range <= 0 )
        {
            m_life = 0;
			m_lastHitByTeamId = m_teamId;
            g_app->GetWorld()->AddOutOfFueldMessage( m_objectId );            
        }
    }

    bool amIDead = MovingObject::Update();
    return amIDead;
}

/*
void Fighter::Render( RenderInfo & renderInfo )
{
    MovingObject::Render( xOffset );
}
*/

void Fighter::RunAI()
{
    if( IsIdle() )
    {
        Land( GetClosestLandingPad() );
    }
}

int Fighter::GetAttackState()
{
    return 0;
}

bool Fighter::UsingGuns()
{
    return true;
}




int Fighter::CountTargettedFighters( int targetId )
{
    int counter = 0;
    for( int i = 0; i < g_app->GetWorld()->m_objects.Size(); ++i )
    {
        if( g_app->GetWorld()->m_objects.ValidIndex(i) )
        {
            WorldObject *obj = g_app->GetWorld()->m_objects[i];
            if( obj->m_type == WorldObject::TypeFighter &&
                obj->m_teamId == m_teamId &&
                obj->m_targetObjectId == targetId )
            {
                counter++;
            }
        }
    }
    return counter;
}


int Fighter::IsValidCombatTarget( int _objectId )
{
    WorldObject *obj = g_app->GetWorld()->GetWorldObject( _objectId );
    if( !obj ) return TargetTypeInvalid;

    if( obj->m_type == TypeCarrier || 
        obj->m_type == TypeAirBase )
    {
        if( obj->m_teamId == m_teamId )
        {
            return TargetTypeLand;
        }
    }

    return MovingObject::IsValidCombatTarget( _objectId );
}

void Fighter::Retaliate( WorldObjectReference const & attackerId )
{
    // fighters are expendable and never retaliate
}

bool Fighter::SetWaypointOnAction()
{
    return true;
}

