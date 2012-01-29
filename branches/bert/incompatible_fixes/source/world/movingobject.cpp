#include "lib/universal_include.h"

#include <math.h>

#include "lib/resource/resource.h"
#include "lib/resource/image.h"
#include "lib/math/math_utils.h"
#include "lib/math/random_number.h"
#include "lib/sound/soundsystem.h"
#include "lib/preferences.h"
#include "lib/gucci/input.h"

#include "app/globals.h"
#include "app/app.h"
#include "app/game.h"

#include "renderer/map_renderer.h"

#include "network/ClientToServer.h"

#include "interface/interface.h"

#include "world/world.h"
#include "world/movingobject.h"
#include "world/blip.h"
#include "world/fleet.h"


MovingObject::MovingObject()
:   WorldObject(),
    m_speed(0),
    m_historyTimer(0),
    m_maxHistorySize(10),
    m_targetLongitude(0),
    m_targetLatitude(0),
    m_movementType(MovementTypeLand),
    m_range(0),
    m_targetNodeId(-1),
    m_finalTargetLongitude(0),
    m_finalTargetLatitude(0),
    m_pathCalcTimer(1),
    // m_targetLongitudeAcrossSeam(0),
    // m_targetLatitudeAcrossSeam(0),
    m_blockHistory(false),
    m_isLanding(-1)
{
}

MovingObject::~MovingObject()
{
    //om_history.EmptyAndDelete();
    //m_movementBlips.EmptyAndDelete();
}


void MovingObject::InitialiseTimers()
{
    WorldObject::InitialiseTimers();

    Fixed gameScale = World::GetGameScale();
    m_speed /= gameScale;
    if( gameScale < 1 && m_range < Fixed::MAX )
    {
        m_range /= gameScale;
    }
}

// returns room inside landing pad for given type
static int RoomInside( WorldObject * pad, int type )
{
    if( type == WorldObject::TypeFighter ) return pad->m_maxFighters - pad->m_states[0]->m_numTimesPermitted;
    if( type == WorldObject::TypeBomber ) return pad->m_maxBombers - pad->m_states[1]->m_numTimesPermitted;
    return 0;
}

void MovingObject::UpdateHistory()
{
    UpdateHistory( 2 );
}

void MovingObject::UpdateHistory( Fixed const & interval )
{
    // adapt history so it does not cross the seam
    {
        float lastX = m_longitude.DoubleValue();
        for( int i = 0; i < m_history.Size(); ++i )
        {
            Vector2< float > & item = *m_history.GetPointer(i);
            float gap = item.x - lastX;
            if( gap < -180 )
            {
                item.x += 360;
            }
            else if( gap > 180 )
            {
                item.x -= 360;
            }
            else
            {
                break;
            }
            lastX = item.x;
        }
    }

    //
    // Update history    
    m_historyTimer -= SERVER_ADVANCE_PERIOD * g_app->GetWorld()->GetTimeScaleFactor() / 10;
    if( m_historyTimer <= 0 )
    {
        m_history.PutDataAtStart( Vector2<float>(m_longitude.DoubleValue(), m_latitude.DoubleValue()) );
        m_historyTimer = interval;
    }

    while( m_maxHistorySize != -1 && 
           m_history.ValidIndex(m_maxHistorySize) )
    {
        m_history.RemoveData(m_maxHistorySize);
    }
}

bool MovingObject::Update()
{
    World * world = g_app->GetWorld();
    
    UpdateHistory();

    for( int i = 0; i < world->m_teams.Size(); ++i )
    {
        Team *team = world->m_teams[i];
        m_lastSeenTime[team->m_teamId] -= world->GetTimeScaleFactor() * SERVER_ADVANCE_PERIOD ;
        if( m_lastSeenTime[team->m_teamId] < 0 ) m_lastSeenTime[team->m_teamId] = 0;
    }

    if( m_isLanding != -1 )
    {
        WorldObject *home = world->GetWorldObject( m_isLanding );
        if( home )
        {
            if( RoomInside( home, m_type ) <= 0 )
            {
                Land( GetClosestLandingPad() );
            }            
            else if( world->GetDistanceSqd( m_longitude, m_latitude, home->m_longitude, home->m_latitude ) < 2 * 2 )
            {
                if( home->m_teamId == m_teamId )
                {
                    bool landed = false;
                    if( m_type == WorldObject::TypeFighter )
                    {
                        if( home->m_states[0]->m_numTimesPermitted < home->m_maxFighters )
                        {
                            home->m_states[0]->m_numTimesPermitted++;
                            landed = true;
                        }
                    }
                    else
                    {
                        if( home->m_states[1]->m_numTimesPermitted < home->m_maxBombers )
                        {
                            home->m_states[1]->m_numTimesPermitted++;
                            home->m_nukeSupply += m_states[1]->m_numTimesPermitted;
                            landed = true;
                        }
                    }
                    if( landed )
                    {
                        m_life = 0;
						m_lastHitByTeamId = -1;
                    }
                    else
                    {
                        Land( GetClosestLandingPad() );
                    }
                }
            }
            else if( home->IsMovingObject() && 
                     ( home->m_longitude != m_targetLongitude || home->m_latitude != m_targetLatitude ) )
            {
                Land( home );
            }
        }
        else
        {
            m_isLanding = -1;
        }
    }

    for( int i = 0; i < world->m_teams.Size(); ++i )
    {
        Team *team = world->m_teams[i];
        if( m_lastSeenTime[team->m_teamId] <= 0 )
        {
            m_seen[team->m_teamId] = false;
        }
    }
 
    return WorldObject::Update();
}

bool MovingObject::IsValidPosition ( Fixed longitude, Fixed latitude )
{
    bool validWaypoint = false;

    if( m_movementType == MovementTypeLand && !g_app->GetMapRenderer()->IsValidTerritory( -1, longitude, latitude, false ) ) validWaypoint = true;
    if( m_movementType == MovementTypeSea && g_app->GetMapRenderer()->IsValidTerritory( -1, longitude, latitude, true ) ) validWaypoint = true;
    if( m_movementType == MovementTypeAir ) validWaypoint = true;

    return validWaypoint;
}

void MovingObject::SetWaypoint( Fixed longitude, Fixed latitude )
{
    if( latitude > 100 || latitude < -100 )
    {
        return;
    }

    // stop the unit before setting a new course
    ClearWaypoints();
       
    if( m_movementType == MovementTypeAir )
    {
        World::SanitiseTargetLongitude( m_longitude, longitude );
    }

    m_targetLongitude = longitude;
    m_targetLatitude = latitude;
}


char *HashDouble( double value, char *buffer );


void FFClamp( Fixed &f, unsigned long long clamp )
{
    union {
        double d;
        unsigned long long l;
    } value;

    value.d = f.DoubleValue();
    value.l &= ~(unsigned long long)clamp;

    f = Fixed::FromDouble( value.d );
}



//void MovingObject::CalculateNewPosition( Fixed *newLongitude, Fixed *newLatitude, Fixed *newDistance )
//{
//    if( m_longitude > -180 && m_longitude < 0 && m_targetLongitude > 180 )
//    {
//        m_targetLongitude -= 360;
//    }
//    else if( m_longitude < 180 && m_longitude > 0 && m_targetLongitude < -180 )
//    {
//        m_targetLongitude += 360;
//    }
//    
//    Vector2<Fixed> targetDir = (Vector2<Fixed>( m_targetLongitude, m_targetLatitude ) -
//								Vector2<Fixed>( m_longitude, m_latitude )).Normalise();
//    Vector2<Fixed> originalTargetDir = targetDir;
//
//    Fixed timePerUpdate = SERVER_ADVANCE_PERIOD * g_app->GetWorld()->GetTimeScaleFactor();
//    
//    Fixed factor1 = m_turnRate * timePerUpdate / 10;
//    Fixed factor2 = 1 - factor1;
//
//    {
//        char buffer1[32];
//        char buffer2[32];
//        SyncRandLog( "a: %s %s\n", HashDouble( m_vel.x.DoubleValue(), buffer1 ), HashDouble( m_vel.y.DoubleValue(), buffer2 ) );
//    }
//
//    m_vel = ( targetDir * factor1 ) + ( m_vel * factor2 );
//
//    FFClamp(m_vel.x, 0xF);
//    FFClamp(m_vel.y, 0xF);
//    FFClamp(m_vel.z, 0xF);
//
//
//    {
//        char buffer1[32];
//        char buffer2[32];
//        char buffer3[32];
//        char buffer4[32];
//        char buffer5[32];
//        char buffer6[32];
//        SyncRandLog( "a1: %s %s %s %s %s %s\n", 
//                            HashDouble( factor1.DoubleValue(), buffer3 ),
//                            HashDouble( factor2.DoubleValue(), buffer4 ),
//                            HashDouble( targetDir.x.DoubleValue(), buffer5 ),
//                            HashDouble( targetDir.y.DoubleValue(), buffer6 ),
//                            HashDouble( m_vel.x.DoubleValue(), buffer1 ), 
//                            HashDouble( m_vel.y.DoubleValue(), buffer2 ) );
//    }
//
//    //m_vel.Normalise();
//
//    Fixed lenSqrd = m_vel.x*m_vel.x + m_vel.y*m_vel.y + m_vel.z*m_vel.z;
//    if (lenSqrd > 0)
//    {
//        Fixed len = sqrt(lenSqrd);
//        Fixed invLen = 1 / len;
//
//        {
//            char buffer1[32];
//            char buffer2[32];
//            char buffer3[32];
//            SyncRandLog( "a2: %s %s %s\n", 
//                            HashDouble( lenSqrd.DoubleValue(), buffer1 ), 
//                            HashDouble( len.DoubleValue(), buffer2 ),
//                            HashDouble( invLen.DoubleValue(), buffer3 ) );
//        }
//
//        m_vel.x *= invLen;
//        m_vel.y *= invLen;
//        m_vel.z *= invLen;
//    }
//    else
//    {
//        m_vel.x = m_vel.y = 0;
//        m_vel.z = 1;
//    }
//
//    FFClamp(m_vel.x, 0xFF);
//    FFClamp(m_vel.y, 0xFF);
//    FFClamp(m_vel.z, 0xFF);
//
//    {
//        char buffer1[32];
//        char buffer2[32];
//        SyncRandLog( "b: %s %s\n", HashDouble( m_vel.x.DoubleValue(), buffer1 ), HashDouble( m_vel.y.DoubleValue(), buffer2 ) );
//    }
//
//	Fixed dotProduct = originalTargetDir * m_vel;
//    Fixed angle = acos( dotProduct );
//
//    if( dotProduct < Fixed::FromDouble(-0.98) )
//    {
//        m_turning = true;
//    }
//
//    if( m_turning )
//    {
//        Fixed turn = (angle / 50) * timePerUpdate;
//        m_vel.RotateAroundZ( turn );
//        m_vel.Normalise();
//        m_angleTurned += turn;
//        if( turn > Fixed::Hundredths(12) )
//        {
//            m_turning = false;
//            m_angleTurned = 0;
//        }
//    }
//
//    m_vel *= m_speed;
//
//    FFClamp(m_vel.x, 0xFFF);
//    FFClamp(m_vel.y, 0xFFF);
//    FFClamp(m_vel.z, 0xFFF);
//
//    {
//        char buffer1[32];
//        char buffer2[32];
//        SyncRandLog( "c: %s %s\n", HashDouble( m_vel.x.DoubleValue(), buffer1 ), HashDouble( m_vel.y.DoubleValue(), buffer2 ) );
//    }
//
//    *newLongitude = m_longitude + m_vel.x * Fixed(timePerUpdate);
//    *newLatitude = m_latitude + m_vel.y * Fixed(timePerUpdate);
//    *newDistance = g_app->GetWorld()->GetDistance( *newLongitude, *newLatitude, m_targetLongitude, m_targetLatitude );
//}


void MovingObject::CalculateNewPosition( Fixed *newLongitude, Fixed *newLatitude )
{
    World::SanitiseTargetLongitude( m_longitude, m_targetLongitude );

    Direction targetDir = (Direction( m_targetLongitude, m_targetLatitude ) -
                           Direction( m_longitude, m_latitude ));
    
    Fixed distanceSquared = targetDir.MagSquared();

    // if you left the map (and have already turned back, code below takes care of that bit),
    // return ASAP, or you're invisible to extreme widescreen users or players who never zoom out.
    if( m_latitude > 100 || m_latitude < -100 )
    {
        Fixed velyout = m_vel.y * ( m_latitude > 0 ? 1 : -1 );
        if( velyout > 0 || -velyout > m_vel.x || -velyout > -m_vel.x )
        {
            // not going in far enough; turn around (as quickly as possible)
            if( velyout >= 0 )
            {
                targetDir.x = 0;
                targetDir.y = -m_latitude;
            }
            else
            {
                targetDir.x = -m_vel.x;
                targetDir.y = 0;
            }

            // make sure the "don't turn too soon" code doesn't get triggered
            distanceSquared = 100000000;
        }
        else if ( m_vel.MagSquared() > 0 )
        {
            // already going back in enough; go straight
            distanceSquared = 0;
        }
    }

    Fixed timePerUpdate = SERVER_ADVANCE_PERIOD * g_app->GetWorld()->GetTimeScaleFactor();
    if( distanceSquared > 0 )
    {
        targetDir.Normalise();

        Fixed dotProduct = targetDir * m_vel;
        Fixed factor1 = m_turnRate * timePerUpdate / 10;

        if( dotProduct < 0 )
        {
            Fixed turnRadius = m_speed / m_turnRate;
            if( m_movementType == MovementTypeAir && distanceSquared < turnRadius * turnRadius )
            {
                // target is not too far behind us, go straight for a bit so we can
                // actually make the turn.
                targetDir.x = 0;
                targetDir.y = 0;
            }
            else
            {
                // we're facing away from the target.
                // make it so that targetDir is projected perpendicularly to
                // m_vel.
                targetDir -= m_vel * (dotProduct/m_vel.MagSquared());

                /* (no longer required, the code at the beginning takes care of everything)
                // pole nonsploit (for lack of term, it's technically an exploit, but
                // pretty useless): by carefully balancing waypoints, you can move a
                // plane as far north of the north pole as its range allows.
                Fixed sign = m_latitude > 0 ? 1 : -1;
                Fixed maxLat = sign * 90;
                if( sign * ( maxLat - m_targetLatitude ) < 0 )
                {
                    maxLat = m_targetLatitude;
                }
                if( ( m_latitude -maxLat ) * sign > 0 && targetDir.y * sign > 0 )
                {
                    // out of sensible bounds and going out, only turn back inward
                    targetDir *= -1;
                }
                */

                if( targetDir.MagSquared() > 0 )
                {
                    // normalize again for maximal turn speed
                    targetDir.Normalise();
                }
                else
                {
                    // we're moving exactly away from the target. Do something random:
                    // a small rotation. Normalization will take care of the rest.
                    m_vel.x += m_vel.y * factor1;
                    m_vel.y -= m_vel.x * factor1;
                }
            }
        }

        // use original code from here on.
        if( factor1 > Fixed::Hundredths(80) )
        {
            factor1 = Fixed::Hundredths(80);
        }
        Fixed factor2 = 1 - factor1;

        m_vel = ( targetDir * factor1 ) + ( m_vel * factor2 );
        m_vel.Normalise();
        m_vel *= m_speed;
    }
        
    *newLongitude = m_longitude + m_vel.x * Fixed(timePerUpdate);
    *newLatitude = m_latitude + m_vel.y * Fixed(timePerUpdate);
}


bool MovingObject::MoveToWaypoint()
{
    Fixed timePerUpdate = SERVER_ADVANCE_PERIOD * g_app->GetWorld()->GetTimeScaleFactor();
    m_range -= Vector2<Fixed>( m_vel.x * Fixed(timePerUpdate), m_vel.y * Fixed(timePerUpdate) ).Mag();
    if( m_targetLongitude != 0 &&
        m_targetLatitude != 0 )
    {
        Fixed distToTarget = g_app->GetWorld()->GetDistance( m_longitude, m_latitude, m_targetLongitude, m_targetLatitude);

        Fixed newLongitude;
        Fixed newLatitude;

        CalculateNewPosition( &newLongitude, &newLatitude );

        // if the unit has reached the edge of the map, move it to the other side and update all nessecery information
        if( newLongitude <= -180 ||
            newLongitude >= 180 )
        {
            m_longitude = newLongitude;
            CrossSeam();
            newLongitude = m_longitude;
        }

        Fixed newDistance = g_app->GetWorld()->GetDistance( newLongitude, newLatitude, m_targetLongitude, m_targetLatitude );

        m_longitude = newLongitude;
        m_latitude = newLatitude;

        if( (newDistance < timePerUpdate * m_speed * Fixed::Hundredths(50)) ||
            (m_movementType == MovementTypeAir &&
             newDistance < 3 &&
             newDistance > distToTarget) ||
            (m_movementType == MovementTypeSea &&
             newDistance < 1 &&
             newDistance > distToTarget))
        {
            ClearWaypoints();
            if( m_movementType == MovementTypeSea )
            {
                m_vel.Zero();
            }
            return false;
        }
        else
        {
            if( m_range <= 0 )
            {
                m_life = 0;
				m_lastHitByTeamId = m_teamId;
                g_app->GetWorld()->AddOutOfFueldMessage( m_objectId );
            }
        }
        return false;
    }
    else
    {
        // just move
        Fixed timePerUpdate = SERVER_ADVANCE_PERIOD * g_app->GetWorld()->GetTimeScaleFactor();
        m_longitude += m_vel.x * Fixed(timePerUpdate);
        m_latitude  += m_vel.y * Fixed(timePerUpdate);
        CrossSeam();
   }

    return true;
    
}

void MovingObject::CrossSeam()
{
    if( m_longitude >= 180 )
    {
        Fixed amountCrossed = m_longitude - 180;
        m_longitude = -180 + amountCrossed;
        m_targetLongitude -= 360;
    }
    else if( m_longitude <= -180 )
    {
        Fixed amountCrossed = m_longitude + 180;
        m_longitude = 180 + amountCrossed;
        m_targetLongitude += 360;
    }
}

void MovingObject::Render( RenderInfo & renderInfo )
{    
    if( m_movementType == MovementTypeAir )
    {
        // already done by map renderer
        // renderInfo.FillPosition(this);

        // float angle = atan( -m_vel.x.DoubleValue() / m_vel.y.DoubleValue() );
        // if( m_vel.y < 0 ) angle += M_PI;
        
        renderInfo.m_direction = renderInfo.m_velocity;
        renderInfo.m_direction /= renderInfo.m_velocity.Mag();

        Vector2< float > displayPos( renderInfo.m_position + renderInfo.m_velocity * 2 );
        float size = GetSize().DoubleValue();

        Team *team = g_app->GetWorld()->GetTeam(m_teamId);
        Colour colour = team->GetTeamColour();
        colour.m_a = 255;
        
        Image *bmpImage = g_resource->GetImage( bmpImageFilename );
        g_renderer->Blit( bmpImage, 
                          renderInfo.m_position.x,
                          renderInfo.m_position.y,
                          size/2, 
                          size/2, 
                          colour, 
                          renderInfo.m_direction.y,
                          -renderInfo.m_direction.x );


        //
        // Current selection?

        colour.Set(255,255,255,255);
        int selectionId = g_app->GetMapRenderer()->GetCurrentSelectionId();
        for( int i = 0; i < 2; ++i )
        {
            if( i == 1 ) 
            {            
                int highlightId = g_app->GetMapRenderer()->GetCurrentHighlightId();
                if( highlightId == selectionId ) break;
                selectionId = highlightId;
            }

            if( selectionId == m_objectId )
            {
                bmpImage = g_resource->GetImage( GetBmpBlurFilename() );
                g_renderer->Blit( bmpImage, 
                                renderInfo.m_position.x,
                                renderInfo.m_position.y,
                                size/2, 
                                size/2, 
                                colour, 
                                renderInfo.m_direction.y,
                                -renderInfo.m_direction.x );

            }
            colour.m_a *= 0.5f;
        }
    }
    else
    {
        WorldObject::Render( renderInfo );
    }

    //
    // Render history
    if( g_preferences->GetInt( PREFS_GRAPHICS_TRAILS ) == 1 )
    {
        RenderHistory( renderInfo );
    }
}

void MovingObject::PrepareRender( RenderInfo & renderInfo )
{
    int sizeCap = (int)(80 * g_app->GetMapRenderer()->GetZoomFactor() );
    sizeCap /= World::GetGameScale().DoubleValue();

    for( int i = NumObjectTypes-1; i >= 0; --i )
    {
        renderInfo.m_maxHistoryRender[i] = sizeCap;
    }

    if( g_app->GetGame()->GetOptionValue("GameMode") == GAMEMODE_BIGWORLD )
    {
        renderInfo.m_maxHistoryRender[TypeNuke] = 12 * g_app->GetMapRenderer()->GetZoomFactor();
        if( g_app->GetMapRenderer()->GetZoomFactor() < 0.25f )
        {
            renderInfo.m_maxHistoryRender[TypeNuke] = 0;
        }

        for( int i = NumObjectTypes-1; i >= 0; --i )
        {
            if( renderInfo.m_maxHistoryRender[i] < 2 )
            {
                renderInfo.m_maxHistoryRender[i] = 0;
            }
        }
    }
}

void MovingObject::RenderHistory( RenderInfo & renderInfo )
{
    int maxSize = m_history.Size();
    int sizeCap = renderInfo.m_maxHistoryRender[m_type];
    maxSize = ( maxSize > sizeCap ? sizeCap : maxSize );

    if( maxSize <= 0 )
    {
        return;
    }

    Team *team = g_app->GetWorld()->GetTeam(m_teamId);
    Colour colour;
    if( team )
    {
        colour = team->GetTeamColour();
    }
    else
    {
        colour = COLOUR_SPECIALOBJECTS;
    }

    int myTeamId = g_app->GetWorld()->m_myTeamId;
    if( m_teamId != myTeamId &&
        myTeamId != -1 &&
        myTeamId < g_app->GetWorld()->m_teams.Size() &&
        g_app->GetWorld()->GetTeam(myTeamId)->m_type != Team::TypeUnassigned )
    {
        maxSize = min( maxSize, 4 );
    }

    Vector2<float> lastPos( renderInfo.m_position );

    for( int i = 0; i < maxSize; ++i )
    {
        Vector2<float> thisPos = m_history[i];;
        thisPos.x += renderInfo.m_xOffset;
        
        Vector2<float> diff = thisPos - lastPos;
        lastPos += diff * 0.1f;
        colour.m_a = 255 - 255 * (float) i / (float) maxSize;
        
        g_renderer->Line( lastPos.x, lastPos.y, thisPos.x, thisPos.y, colour, 2.0f );        
        lastPos = thisPos;
    }
}


int MovingObject::GetAttackState()
{
    return -1;
}

bool MovingObject::IsIdle()
{
    if( m_targetLongitude == 0 &&
        m_targetLatitude  == 0 &&
        m_targetObjectId == -1)
        return true;
    else return false;
}


void MovingObject::RenderGhost( int teamId, RenderInfo & renderInfo )
{
    if( m_lastSeenTime[teamId] != 0)
    {
        if( m_movementType == MovementTypeAir ||
            m_movementType == MovementTypeSea )
        {		
            renderInfo.m_position.x = m_longitude.DoubleValue()+renderInfo.m_xOffset;
            renderInfo.m_position.y = m_latitude.DoubleValue();
            Vector2< float > vel( m_vel.x.DoubleValue(), m_vel.y.DoubleValue() );
            renderInfo.m_position += vel * renderInfo.m_predictionTime;

            float size = GetSize().DoubleValue();       
            float thisSize = size;

            int transparency = int(255 * ( m_lastSeenTime[teamId] / m_ghostFadeTime ).DoubleValue() );
            Colour col = Colour(150, 150, 150, transparency);
            float angle = 0.0f;

            if( m_movementType == MovementTypeAir )            
            {
                angle = atan( -m_lastKnownVelocity[teamId].x.DoubleValue() / m_lastKnownVelocity[teamId].y.DoubleValue() );
                if( m_lastKnownVelocity[teamId].y < 0 ) angle += M_PI;
                size *= 0.5f;
                thisSize *= 0.5f;
            }
            else
            {
                Team *team = g_app->GetWorld()->GetTeam(m_teamId);
                if( team->m_territories[0] >= 3 )
                {
                    thisSize = size*-1;
                }       
            }

            Image *bmpImage = g_resource->GetImage( bmpImageFilename );
            g_renderer->Blit( bmpImage, renderInfo.m_position.x, renderInfo.m_position.y, thisSize, size, col, angle);
        }
        else
        {
            WorldObject::RenderGhost( teamId, renderInfo );
        }
    }
}

void MovingObject::Land( WorldObject * target )
{
    if( target )
    {
        Fixed  interceptLongitude, interceptLatitude;
        GetInterceptionPoint( target, &interceptLongitude, &interceptLatitude );

        if( RoomInside( target, m_type ) <= 0 )
        {
            // get distance
            Fixed distance = g_app->GetWorld()->GetDistance( m_longitude, m_latitude, interceptLongitude, interceptLatitude );
            // landing pad is full. Just go towards there, but don't land.
            Fixed middleLongitude = (m_longitude+interceptLongitude)/2;
            Fixed middleLatitude =  (m_latitude+interceptLatitude)/2;

            if( distance*2 > m_range+5 )
            {
                // we're kind of far away still, go directly
                SetWaypoint( middleLongitude, middleLatitude );
            }
            else
            {
                // we're close. muck about a little.
                Fixed range = m_range/16+distance/16+5;
                SetWaypoint( middleLongitude + range*(syncfrand(2)-1), middleLatitude + range*(syncfrand(2)-1) );
            }
        }
        else
        {
            SetWaypoint( interceptLongitude, interceptLatitude );
            m_isLanding = target;
        }
    }
    else
    {
        m_isLanding = -1;
    }
}

void MovingObject::ClearWaypoints()
{
    m_targetLongitude = 0;
    m_targetLatitude = 0;
    m_finalTargetLongitude = 0;
    m_finalTargetLatitude = 0;
    // m_targetLongitudeAcrossSeam = 0;
    // m_targetLatitudeAcrossSeam = 0;
    m_targetNodeId = -1;
    m_isLanding = -1;
//    m_movementBlips.EmptyAndDelete();
}


int MovingObject::IsValidMovementTarget( Fixed longitude, Fixed latitude )
{
    if( latitude > 100 || latitude < -100 )
    {
        return TargetTypeInvalid;
    }

    //
    // Are we part of a fleet?

    Fleet *fleet = g_app->GetWorld()->GetTeam(m_teamId)->GetFleet( m_fleetId );
    if( fleet )
    {
        return( fleet->IsValidFleetPosition( longitude, latitude ) );
    }


    Fixed rangeSqd = g_app->GetWorld()->GetDistanceSqd( m_longitude, m_latitude, longitude, latitude );      
    if( rangeSqd > m_range * m_range )
    {
        return TargetTypeOutOfRange;
    }

    bool validPosition = IsValidPosition( longitude, latitude );

    if( validPosition )
    {
        return TargetTypeValid;
    }
    else
    {
        return TargetTypeInvalid;
    }
}


void MovingObject::AutoLand()
{
    if( m_isLanding == -1 )
    {
        WorldObject * landingPad = GetClosestLandingPad();

        if( landingPad )
        {
            if( m_range - 15 < g_app->GetWorld()->GetDistance( m_longitude, m_latitude, landingPad->m_longitude, landingPad->m_latitude) )
            {
                Land( landingPad );
            }
        }
    }
}

bool MovingObject::GetClosestLandingPad( BoundedArray<int> const & alreadyLanding, BoundedArray<int> const & alreadyLandingWantingNukes, Fixed const & turnRadius, WorldObject * & goFor, WorldObject * & nearestNonViable )
{
    WorldObject * nearest = NULL;
    Fixed nearestSqd = Fixed::MAX;

    nearestNonViable = NULL;
    Fixed nearestNonViableDistSqd = Fixed::MAX;
    int nearestNonViableRoom = -100;

    // take into account that we may need to turn around
    Fixed range = m_range - turnRadius;
    Fixed turnLongitude = m_longitude + m_vel.x * turnRadius;
    Fixed turnLatitude  = m_latitude  + m_vel.y * turnRadius;

    // for the nearest nuke supply, prefer larger supplies and don't look further than
    // our remaining fuel range
    Fixed rangeSqd = range * range;
    WorldObject * nearestWithNukes = NULL;
    Fixed nearestWithNukesSqd = rangeSqd;
    int maxNukeSupply = 1;

    //
    // Look for any carrier or airbase with room
    // Favour objects with nukes if we are a bomber
    
    int maxI = g_app->GetWorld()->m_objects.Size();

    // optimization hack exploiting that the object array never reassigns indices:
    // we don't actually need to iterate over all objects, just the first few.
    if( alreadyLanding.Size() < maxI )
    {
        maxI = alreadyLanding.Size();
    }
    for( int i = 0; i < maxI; ++i )
    {
        if( g_app->GetWorld()->m_objects.ValidIndex(i) )
        {
            WorldObject *obj = g_app->GetWorld()->m_objects[i];
            if( obj->m_teamId == m_teamId )
            {
                if( obj->m_type == WorldObject::TypeCarrier ||
                    obj->m_type == WorldObject::TypeAirBase )
                {
                    int roomInside = RoomInside( obj, m_type ) - alreadyLanding[obj->m_objectId];

                    Fixed distSqd = g_app->GetWorld()->GetDistanceSqd( turnLongitude, turnLatitude, obj->m_longitude, obj->m_latitude );
                    if( roomInside > 0 )
                    {
                        if( distSqd < nearestSqd )
                        {
                            nearestSqd = distSqd;
                            nearest = obj;
                        }

                        if( m_type == TypeBomber && obj->m_nukeSupply > 0 )
                        {
                            // must be an airbase or carrier
                            AppAssert( obj->m_type == TypeCarrier || obj->m_type == TypeAirBase );

                            // get nukes left to distribute: nukes minus bombers
                            int nukesLeft = obj->m_nukeSupply - obj->m_states[1]->m_numTimesPermitted - alreadyLandingWantingNukes[obj->m_objectId];

                            // closer is better, more nukes is even better
                            if ( ( distSqd < nearestWithNukesSqd && nukesLeft >= maxNukeSupply ) ||
                                 ( distSqd < rangeSqd && nukesLeft > maxNukeSupply ) ) 
                            {
                                nearestWithNukesSqd = distSqd;
                                nearestWithNukes = obj;
                                maxNukeSupply = nukesLeft;
                            }
                        }
                    }
                    else
                    {
                        // last ditch search
                        bool better = false;
                        
                        // absolutely prefer objects in range
                        bool newInRange = distSqd < range*range;
                        bool oldInRange = nearestNonViableDistSqd < range*range;
                        if( newInRange && !oldInRange )
                        {
                            better = true;
                        }
                        else if ( newInRange == oldInRange )
                        {
                            // prefer less overbooked spots
                            if( roomInside > nearestNonViableRoom )
                            {
                                better = true;
                            }
                            else if( roomInside == nearestNonViableRoom )
                            {
                                // prefer closer spots
                                better = distSqd < nearestNonViableDistSqd;
                            }
                        }

                        if ( better )
                        {
                            nearestNonViableDistSqd = distSqd;
                            nearestNonViable = obj;
                            nearestNonViableRoom = roomInside;
                        }
                    }
                }
            }
        }
    }

    goFor = NULL;

    //
    // Bomber without nuke - go for nearest with nukes if in range

    if( m_type == TypeBomber && 
        m_states[1]->m_numTimesPermitted == 0 &&
        nearestWithNukes &&
        nearestWithNukesSqd < m_range * m_range )
    {
        goFor = nearestWithNukes;
    }

    //
    // Fighter or bomber with nuke or no nuke supply in range: go for nearest

    if( !goFor )
    {
        goFor = nearest;
    }

    //
    // Nothing found? Nothing at all? Shoot, go for the nearest whatever.

    if( !goFor )
    {
        goFor = nearestNonViable;
    }

    // 
    // nothing at all? Give up.
    if( !goFor )
    {
        return false;
    }

    // check whether we will have enough fuel to make it
    Fixed fuelLeft = range - g_app->GetWorld()->GetDistance( turnLongitude, turnLatitude, goFor->m_longitude, goFor->m_latitude );

    return fuelLeft > 0;
}

WorldObject * MovingObject::GetClosestLandingPad()
{
    World * world = g_app->GetWorld();

    // overestimate turn radius it a tiny bit
    // because planes don't exactly turn optimally
    Fixed turnRadius = Fixed::Hundredths(110)*m_speed/m_turnRate;

    // find the highest ID of a landing pad
    int maxLandingPadId = 0;
    for( int i = 0; i < world->m_objects.Size(); ++i )
    {
        if( world->m_objects.ValidIndex(i) )
        {
            WorldObject *obj = world->m_objects[i];
            if( ( obj->m_type == TypeCarrier || obj->m_type == TypeAirBase ) && obj->m_teamId == m_teamId )
            {
                if( obj->m_objectId > maxLandingPadId )
                {
                    maxLandingPadId = obj->m_objectId;
                }
            }
        }
    }

    // count how many other planes of this type are already landing on each platform
    BoundedArray< int > alreadyLanding, alreadyLandingWantingNukes;
    alreadyLanding.Initialise( maxLandingPadId+1 );
    alreadyLanding.SetAll( 0 );
    alreadyLandingWantingNukes.Initialise( maxLandingPadId+1 );
    alreadyLandingWantingNukes.SetAll( 0 );

    for( int i = 0; i < world->m_objects.Size(); ++i )
    {
        if( world->m_objects.ValidIndex(i) )
        {
            WorldObject *obj = world->m_objects[i];
            if( obj != this && obj->m_type == m_type && obj->m_teamId == m_teamId )
            {
                MovingObject * mobj = dynamic_cast< MovingObject * >( obj );
                AppAssert( mobj );
                if( mobj->m_isLanding >= 0 )
                {
                    alreadyLanding[mobj->m_isLanding]++;
                    if( mobj->m_type == TypeBomber && mobj->m_states[1]->m_numTimesPermitted == 0 )
                    {
                        alreadyLandingWantingNukes[mobj->m_isLanding]++;
                    }
                }
            }
        }
    }

    // delegate to core function
    WorldObject * goFor;
    WorldObject * nearestNonViable; // nearest pad, but it's already full
    if( GetClosestLandingPad( alreadyLanding, alreadyLandingWantingNukes, turnRadius, goFor, nearestNonViable ) )
    {
        // it found a good spot for us, take it.
        return goFor;
    }
    
    // no. See if we can steal the landing spot from another plane.
    MovingObject * victim= NULL;
    Fixed maxScore = -Fixed::MAX;
    WorldObject * victimEvade = NULL;
    
    for( int i = 0; i < world->m_objects.Size(); ++i )
    {
        if( world->m_objects.ValidIndex(i) )
        {
            WorldObject *obj = world->m_objects[i];
            if( obj != this && obj->m_type == m_type && obj->m_teamId == m_teamId )
            {
                MovingObject * otherPlane = dynamic_cast< MovingObject * >( obj );
                AppAssert( otherPlane );
                
                // is it landing? only then we can push it away.
                if( otherPlane->m_isLanding >= 0 )
                {
                    // no need to recalculate turn radius, it's of the same type
                    Fixed turnLongitude = otherPlane->m_longitude + otherPlane->m_vel.x * turnRadius;
                    Fixed turnLatitude  = otherPlane->m_latitude  + otherPlane->m_vel.y * turnRadius;
                    
                    WorldObject *landing = world->GetWorldObject(otherPlane->m_isLanding);
                    if( RoomInside( landing, m_type ) - alreadyLanding[landing->m_objectId] < 0 )
                    {
                        // overbooked already.
                        continue;
                    }
                    
                    // get distances to target
                    Fixed distSqd = world->GetDistanceSqd( m_longitude, m_latitude, landing->m_longitude, landing->m_latitude );
                    Fixed dist = sqrt(distSqd);
                    Fixed otherDistSqd = world->GetDistanceSqd( turnLongitude, turnLatitude, landing->m_longitude, landing->m_latitude );

                    Fixed otherDist = sqrt(otherDistSqd);

                    // calculate other plane's fuel supply on landing
                    Fixed otherFuel = otherPlane->m_range - turnRadius - otherDist;

                    Fixed score = otherFuel - otherDist*Fixed::Hundredths(10);
                    // Fixed score = -dist - otherDist*Fixed::Hundredths(1);

                    if( m_type == TypeBomber )
                    {
                        if ( m_states[1]->m_numTimesPermitted > 0 )
                        {
                            // this bomber is carrying a nuke, but try to 
                            // steal the landing spot from a no-nuke bomber. It presumably
                            // hasn't selected an ideal landing spot anyway.
                            score -= 1000 * otherPlane->m_states[1]->m_numTimesPermitted;
                        }
                        else
                        {
                            // this bomber has no nuke, the other has one. Don't pontentially
                            // go waste a nuke, find another victim.
                            if( otherPlane->m_states[1]->m_numTimesPermitted > 0 )
                            {
                                continue;
                            }
                        }
                    }

                    // early exit if the max score can't be beaten
                    int evadeScore = 2000;
                    if( maxScore > score + evadeScore )
                    {
                        continue;
                    }
                    // prefer planes that have a place to evade to
                    WorldObject * evade = NULL;
                    WorldObject * dummy;
                    if( otherPlane->GetClosestLandingPad( alreadyLanding, alreadyLandingWantingNukes, turnRadius, evade, dummy ) )
                    {
                        score += evadeScore;
                    }
                    else
                    {
                        evade = NULL;
                    }

                    if( maxScore > score )
                    {
                        continue;
                    }

                    // calculate this plane's fuel supply pessimistically and our supply 
                    // optimistically to avoid it stealing the spot right back
                    Fixed ourFuelOptimistic = m_range - dist;
                    
                    // only steal spots if the other plane is better off than we are
                    // and if we can save ourselves
                    if( otherFuel > ourFuelOptimistic && ourFuelOptimistic > 0 )
                    {
                        maxScore = score;
                        victim = otherPlane;
                        victimEvade = evade;
                    }
                }
            }
        }
    }

    if( victim )
    {
        // found a victim. Tell it to bugger off and steal its spot.
        goFor = world->GetWorldObject( victim->m_isLanding );
        victim->ClearWaypoints();
        if( victimEvade )
        {
            victim->Land( victimEvade );
        }

        return goFor;
    }
    else
    {
        // no landing spot free. Head for nearest base, hope it gets free when we get there.
        if( nearestNonViable )
        {
            return nearestNonViable;
        }
        else
        {
            return goFor;
        }
    }
}


//int MovingObject::GetClosestLandingPad()
//{
//    int type = -1;
//    int cap = 0;
//    if( m_type == TypeFighter )
//    {
//        type = 0;
//    }
//    else if( m_type == TypeBomber )
//    {
//        type = 1;
//    }
//
//    float carrierDistance = 9999.9f;
//    float airbaseDistance = 9999.9f;
//    int nearestCarrier = -1;
//    int nearestAirbase = -1;
//
//    for( int i = 0; i < g_app->GetWorld()->m_objects.Size(); ++i )
//    {
//        if( g_app->GetWorld()->m_objects.ValidIndex(i) )
//        {
//            WorldObject *obj = g_app->GetWorld()->m_objects[i];
//            if( obj->m_teamId == m_teamId )
//            {
//                if( obj->m_type == WorldObject::TypeCarrier ||
//                    obj->m_type == WorldObject::TypeAirBase )
//                {
//                    if( type == 0 )
//                    {
//                        cap = obj->m_maxFighters;
//                    }
//                    else
//                    {
//                        cap = obj->m_maxBombers;
//                    }
//
//                    if( obj->m_states[type]->m_numTimesPermitted < cap )
//                    {
//                        float dist = g_app->GetWorld()->GetDistance( m_longitude, m_latitude, obj->m_longitude, obj->m_latitude );
//                        if( obj->m_type == WorldObject::TypeCarrier )
//                        {
//                            if( dist < carrierDistance )
//                            {
//                                carrierDistance = dist;
//                                nearestCarrier = obj->m_objectId;
//                            }
//                        }
//                        else
//                        {
//                            if( dist < airbaseDistance )
//                            {
//                                airbaseDistance = dist;
//                                nearestAirbase = obj->m_objectId;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//
//    int target = -1;
//    float distance = 99999.9f;
//
//    WorldObject *base = g_app->GetWorld()->GetWorldObject(nearestAirbase);
//    if( base )
//    {
//        distance = g_app->GetWorld()->GetDistance( m_longitude, m_latitude, base->m_longitude, base->m_latitude );
//        target = nearestAirbase;
//    }
//
//    
//    WorldObject *carrier = g_app->GetWorld()->GetWorldObject(nearestCarrier);
//    if( carrier )
//    {
//        if( g_app->GetWorld()->GetDistance( m_longitude, m_latitude, carrier->m_longitude, carrier->m_latitude ) < distance &&
//            carrier->m_states[type]->m_numTimesPermitted < cap )
//        {
//            target = nearestCarrier;
//        }
//    }
//
//    return target;
//}

// calculates the best target point to intercept the other object
void MovingObject::GetInterceptionPoint( WorldObject *target, Fixed *interceptLongitude, Fixed *interceptLatitude )
{
    Fixed timeLimit = Fixed::MAX;
    Direction targetVel = target->m_vel;

    Fixed targetLongitude = target->m_longitude;
    World::SanitiseTargetLongitude( m_longitude, targetLongitude );

    Direction distance( targetLongitude - m_longitude, target->m_latitude - m_latitude );
    
    // we know exactly where a friendly object is heading
    if( target->m_teamId == m_teamId && target->IsMovingObject() )
    {
        MovingObject * movingTarget = dynamic_cast< MovingObject * >( target );
        AppAssert( movingTarget );
        if ( movingTarget->m_targetLatitude != 0 || movingTarget->m_targetLongitude != 0 ) 
        {
            // assume it's going in a straight line at top speed
            Fixed targetTargetLongitude = movingTarget->m_targetLongitude;
            World::SanitiseTargetLongitude( targetLongitude, targetTargetLongitude );
            targetVel.x = targetTargetLongitude - targetLongitude;
            targetVel.y = movingTarget->m_targetLatitude - movingTarget->m_latitude;
            Fixed distSqd = targetVel.MagSquared();
            if( distSqd > 0 )
            {
                Fixed dist = sqrt( distSqd );
                timeLimit = dist/movingTarget->m_speed;
                targetVel /= timeLimit;
            }
            else
            {
                targetVel = target->m_vel;
            }
        }
    }

    Fixed targetSpeedSqd = targetVel.MagSquared();
    Fixed thisSpeedSqd = m_speed*m_speed;
    if( targetSpeedSqd >= thisSpeedSqd * Fixed::Hundredths(90) )
    {
        // clamp target speed, pretend we can catch up
        targetSpeedSqd = thisSpeedSqd * Fixed::Hundredths(90);
    }

    // calculation of time to rendez-vous
    Fixed dv = 1/(thisSpeedSqd - targetSpeedSqd);
    Fixed dot = distance * targetVel;
    Fixed p = dot*dv;
    Fixed timeLeft = p + sqrt( p*p + distance.MagSquared()*dv );
    if( timeLeft > timeLimit )
    {
        timeLeft = timeLimit;
    }

    // calculate position
    *interceptLongitude = targetLongitude     + timeLeft*targetVel.x;
    *interceptLatitude  = target->m_latitude  + timeLeft*targetVel.y;
}

void MovingObject::Retaliate( WorldObjectReference const & attackerId )
{
    WorldObject *obj = g_app->GetWorld()->GetWorldObject( attackerId );
    if( obj && m_teamId != obj->m_teamId &&
        !g_app->GetWorld()->GetTeam( m_teamId )->m_ceaseFire[ obj->m_teamId ])
    {
        if( UsingGuns() && m_targetObjectId == -1 )
        {            
            if( g_app->GetWorld()->GetAttackOdds( m_type, obj->m_type ) > 0 )
            {
                m_targetObjectId = attackerId;
                m_isRetaliating = true;
            }

            if( m_fleetId != -1 )
            {
                g_app->GetWorld()->GetTeam( m_teamId )->m_fleets[ m_fleetId ]->FleetAction( m_targetObjectId );
            }
        }
        if( obj->m_type == WorldObject::TypeSub )
        {
            Fleet *fleet = g_app->GetWorld()->GetTeam( m_teamId )->GetFleet( m_fleetId );
            if( fleet )
            {
                fleet->CounterSubs();
            }
        }
    }

}

void MovingObject::Ping()
{   
    int animId = g_app->GetMapRenderer()->CreateAnimation( MapRenderer::AnimationTypeSonarPing, m_objectId,
														   m_longitude.DoubleValue(), m_latitude.DoubleValue() );
    AnimatedIcon *icon = g_app->GetMapRenderer()->m_animations[animId];
    ((SonarPing *)icon)->m_teamId = m_teamId;
    
    if( g_app->GetWorld()->m_myTeamId == -1 ||
        g_app->GetWorld()->m_myTeamId == m_teamId ||
        g_app->GetMapRenderer()->m_renderEverything ||
        g_app->GetWorld()->IsVisible( m_longitude, m_latitude, g_app->GetWorld()->m_myTeamId) )
    {
        g_soundSystem->TriggerEvent( SoundObjectId(m_objectId), "SonarPing" );
    }
    

    Fleet *fleet = g_app->GetWorld()->GetTeam( m_teamId )->GetFleet( m_fleetId );
    if( fleet == NULL ||
        fleet->m_currentState == Fleet::FleetStateAggressive )
    {
        for( int i = 0; i < g_app->GetWorld()->m_objects.Size(); ++i )
        {
            if( g_app->GetWorld()->m_objects.ValidIndex(i) )
            {
                MovingObject *obj = (MovingObject *)g_app->GetWorld()->m_objects[i];
                if( obj->IsMovingObject() &&
                    obj->m_movementType == MovementTypeSea &&
                    obj->m_objectId != m_objectId &&
                    !obj->m_visible[ m_teamId ] &&
                    m_teamId != obj->m_teamId )
                {
                    Fixed distanceSqd = g_app->GetWorld()->GetDistanceSqd( m_longitude, m_latitude, obj->m_longitude, obj->m_latitude );

                    if( distanceSqd <= GetActionRangeSqd() )
                    {
                        if( obj->m_movementType == MovementTypeSea )
                        {
                            for( int j = 0; j < g_app->GetWorld()->m_teams.Size(); ++j )
                            {
                                Team *team = g_app->GetWorld()->m_teams[j];
                                if( g_app->GetWorld()->IsFriend(team->m_teamId, m_teamId ) )
                                {
                                    obj->m_lastSeenTime[team->m_teamId] = obj->m_ghostFadeTime;
                                    obj->m_lastKnownPosition[team->m_teamId] = Vector2<Fixed>( obj->m_longitude, obj->m_latitude );                                    
                                    //obj->m_visible[team->m_teamId] = (obj->m_type != WorldObject::TypeSub );
                                    obj->m_seen[team->m_teamId] = true;
                                    obj->m_lastKnownVelocity[team->m_teamId].Zero();
                                }
                            }
                        }

                        if( m_type == WorldObject::TypeSub )
                        {
                            for( int j = 0; j < g_app->GetWorld()->m_teams.Size(); ++j )
                            {
                                Team *team = g_app->GetWorld()->m_teams[j];
                                if( g_app->GetWorld()->IsVisible( m_longitude, m_latitude, team->m_teamId ) )
                                {
                                    m_seen[team->m_teamId] = true;
                                    m_lastSeenTime[team->m_teamId] = m_ghostFadeTime;
                                    m_lastKnownPosition[team->m_teamId] = Vector2<Fixed>( m_longitude, m_latitude );
                                    m_lastKnownVelocity[team->m_teamId].Zero();
                                }
                            }
                        }

                        if( !g_app->GetWorld()->GetTeam( m_teamId )->m_ceaseFire[ obj->m_teamId ] )
                        {
                            if( m_targetObjectId == -1 )
                            {
                                m_targetObjectId = obj->m_objectId;                  
                            }
                            if( g_app->GetWorld()->GetTeam( m_teamId )->m_type == Team::TypeAI )
                            {
                                if( obj->m_type == WorldObject::TypeSub )
                                {
                                    g_app->GetWorld()->GetTeam( m_teamId )->AddEvent( Event::TypeSubDetected, obj->m_objectId, obj->m_teamId, obj->m_fleetId, obj->m_longitude, obj->m_latitude );
                                }
                                else
                                {
                                    g_app->GetWorld()->GetTeam( m_teamId )->AddEvent( Event::TypeEnemyIncursion, obj->m_objectId, obj->m_teamId, obj->m_fleetId, obj->m_longitude, obj->m_latitude );
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void MovingObject::SetSpeed( Fixed speed )
{
    m_speed = speed;
}

int MovingObject::GetTarget( Fixed range )
{
    World * world = g_app->GetWorld();

    LList<int> farTargets;
    LList<int> closeTargets;
    for( int i = 0; i < world->m_objects.Size(); ++i )
    {
        if( world->m_objects.ValidIndex(i) )
        {
            WorldObject *obj = world->m_objects[i];
            bool valid = false;
            if( obj->m_teamId != TEAMID_SPECIALOBJECTS )
            {
                if( !world->IsFriend( obj->m_teamId, m_teamId ) &&
                    obj->m_visible[m_teamId] &&
                    !world->GetTeam( m_teamId )->m_ceaseFire[ obj->m_teamId ] )
                {
                    valid = true;
                }
            }
            else
            {
                valid = obj->m_type == TypeSaucer;
            }
            if( !world->GetAttackOdds( m_type, obj->m_type ) > 0 )
            {
                valid = false;
            }

            if( valid )
            {
                Fixed distanceSqd = world->GetDistanceSqd( m_longitude, m_latitude, obj->m_longitude, obj->m_latitude );
                if( distanceSqd < GetActionRangeSqd() )
                {
                    closeTargets.PutData(obj->m_objectId );
                }
                else if( distanceSqd < range * range )
                {
                    farTargets.PutData( obj->m_objectId );
                }
            }
        }
    }
    if( closeTargets.Size() > 0 )
    {
        int targetIndex = syncrand() % closeTargets.Size();
        return closeTargets[ targetIndex ];
    }
    else if( farTargets.Size() > 0 )
    {
        int targetIndex = syncrand() % farTargets.Size();
        return farTargets[ targetIndex ];
    }
    return -1;
}


char *MovingObject::LogState()
{
    char longitude[64];
    char latitude[64];
    char velX[64];
    char velY[64];
    char timer[64];
    char retarget[64];
    char ai[64];
    char speed[64];
    char targetLong[64];
    char targetLat[64];
    char seenTime[64];

    World * world = g_app->GetWorld();

    static char s_result[10240];
    snprintf( s_result, 10240, "obj[%d] [%10s] team[%d] fleet[%d] long[%s] lat[%s] velX[%s] velY[%s] state[%d] target[%d] life[%d] timer[%s] retarget[%s] ai[%s] speed[%s] targetNode[%d] targetLong[%s] targetLat[%s]",
                (int)m_objectId,
                GetName(m_type),
                m_teamId,
                m_fleetId,
                HashDouble( m_longitude.DoubleValue(), longitude ),
                HashDouble( m_latitude.DoubleValue(), latitude ),
                HashDouble( m_vel.x.DoubleValue(), velX ),
                HashDouble( m_vel.y.DoubleValue(), velY ),
                m_currentState,
                (int)m_targetObjectId,
                m_life,
                HashDouble( m_stateTimer.DoubleValue(), timer ),
                HashDouble( m_retargetTimer.DoubleValue(), retarget ),
                HashDouble( m_aiTimer.DoubleValue(), ai ),
                HashDouble( m_speed.DoubleValue(), speed ),
                m_targetNodeId,
                HashDouble( m_targetLongitude.DoubleValue(), targetLong ),
                HashDouble( m_targetLatitude.DoubleValue(), targetLat ) );

    for( int i = 0; i < world->m_teams.Size(); ++i )
    {
        Team *team = world->m_teams[i];
        char thisTeam[512];
        sprintf( thisTeam, "\n\tTeam %d visible[%d] seen[%d] pos[%s %s] vel[%s %s] seen[%s] state[%d]",
            team->m_teamId,
            m_visible[team->m_teamId],
            m_seen[team->m_teamId],
            HashDouble( m_lastKnownPosition[team->m_teamId].x.DoubleValue(), longitude ),
            HashDouble( m_lastKnownPosition[team->m_teamId].y.DoubleValue(), latitude ),
            HashDouble( m_lastKnownVelocity[team->m_teamId].x.DoubleValue(), velX ),
            HashDouble( m_lastKnownVelocity[team->m_teamId].y.DoubleValue(), velY ),
            HashDouble( m_lastSeenTime[team->m_teamId].DoubleValue(), seenTime ),
            m_lastSeenState[team->m_teamId] );

        strcat( s_result, thisTeam );
    }

    return s_result;
}

