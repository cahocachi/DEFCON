#include "lib/universal_include.h"
#include "lib/resource/resource.h"
#include "lib/resource/image.h"
#include "lib/render/renderer.h"
#include "lib/math/vector3.h"
#include "lib/math/random_number.h"
#include "lib/language_table.h"

#include "app/app.h"
#include "app/globals.h"

#include "interface/interface.h"

#include "world/world.h"
#include "world/saucer.h"
#include "world/team.h"
#include "world/city.h"

static MovingUnitSettings s_saucerSettings( WorldObject::TypeSaucer, 50, 20, 10, 200000 );
static StateSettings s_saucerFlight( WorldObject::TypeSaucer, "Flight", 0, 0, 10, 180, true );
static StateSettings s_saucerAttack( WorldObject::TypeSaucer, "Attack", 0, 0, 10, 180, true );

Saucer::Saucer()
:   MovingObject(),
    m_leavingWorld(false)
{
    Setup( TypeSaucer, s_saucerSettings );

    strcpy( bmpImageFilename, "graphics/saucer.bmp" );

    m_radarRange = 10;
    m_selectable = true;
    m_maxHistorySize = 10;

    m_explosionSize = Fixed::Hundredths(5);
    m_damageTimer = 20;
    m_angle = 0.0f;

    m_movementType = MovementTypeAir;

    AddState( LANGUAGEPHRASE("state_flight"), s_saucerFlight );
    AddState( LANGUAGEPHRASE("state_attack"), s_saucerAttack );

    InitialiseTimers();
}


void Saucer::Action( WorldObjectReference const & targetObjectId, Fixed longitude, Fixed latitude )
{
    if( m_currentState == 0 )
    {
        SetWaypoint( longitude, latitude );
    }
    else if( m_currentState == 1 )
    {
       
    }    
}

bool Saucer::Update()
{
    //
    // Do we move ?

    if( m_leavingWorld )
    {
        return true;
    }

    if( m_currentState == 0 )
    {    
        Direction oldVel = m_vel;

        bool arrived = MoveToWaypoint();
        if( arrived )
        {
            if( m_targetObjectId >= 0 )
            {
                m_vel.x = m_vel.y = 0;
                m_targetLongitude = 0;
                m_targetLatitude = 0;
                int targetId = m_targetObjectId;
                SetState( 1 );    
                m_targetObjectId = targetId;
            }
            else
            {
                GetNewTarget();
            }
        }
    }
    else 
    {
        if( g_app->GetWorld()->m_cities.ValidIndex( m_targetObjectId ) )
        {
            m_explosionSize += Fixed::Hundredths(5);
            if( m_damageTimer <= 0 )
            {
                City *city = g_app->GetWorld()->m_cities[m_targetObjectId];
                if( city->m_population > 0 )
                {
                    int deaths = (city->m_population * (m_explosionSize / 10 )).IntValue(); 
                    if( deaths > 0 )
                    {
                        Team *owner = g_app->GetWorld()->GetTeam( city->m_teamId );
                        if( owner )
                        {
                            owner->m_friendlyDeaths += deaths;
                        }
                        city->m_population -= deaths;
                        if ( city->m_population < 1000 )
                        {
                            city->m_population = 0;
                        }
                        /*
                        char caption[256];
                        sprintf( caption, "ALIEN ATTACK on %s, %u dead", city->m_name, deaths );
                        g_app->GetInterface()->ShowMessage( m_longitude, m_latitude, TEAMID_SPECIALOBJECTS, caption, false);
                        */
                        m_damageTimer = 20;
                    }
                }
                else
                {
                    char msg[256];
                    sprintf( msg, LANGUAGEPHRASE("alien_attack") );
                    LPREPLACESTRINGFLAG( 'C', LANGUAGEPHRASEADDITIONAL(city->m_name), msg );

                    g_app->GetWorld()->AddWorldMessage( city->m_longitude, city->m_latitude, TEAMID_SPECIALOBJECTS, msg, WorldMessage::TypeDirectHit);
                    GetNewTarget();
                }
            }
            else
            {
                m_damageTimer -= SERVER_ADVANCE_PERIOD * g_app->GetWorld()->GetTimeScaleFactor();
                if( m_damageTimer < 0 )
                    m_damageTimer = 0;
            }
        }
        else
        {
            GetNewTarget();
        }
    }

    return MovingObject::Update();   
}

void Saucer::Render( RenderInfo & renderInfo )
{
    renderInfo.FillPosition(this);

    float size = 8.0f;

    Colour colour       = COLOUR_SPECIALOBJECTS;
    if( m_currentState == 0 )
    {
        RenderHistory( renderInfo ); 
    }
    
    m_angle += 0.01f;

    Image *bmpImage = g_resource->GetImage( bmpImageFilename );
    if( m_currentState == 0 )
    {  
        g_renderer->Blit( bmpImage, renderInfo.m_position.x + renderInfo.m_velocity.x * 2,
						  renderInfo.m_position.y + renderInfo.m_velocity.y * 2, size/2, size/2, colour, m_angle );
    }
    else
    {
        g_renderer->Blit( bmpImage, renderInfo.m_position.x, renderInfo.m_position.y, size/2, size/2, colour, m_angle );
    }
    
    if( m_currentState == 1 )
    {
        Image *explosion = g_resource->GetImage( "graphics/explosion.bmp" );
		float explosionSize = m_explosionSize.DoubleValue();
		Colour fire = Colour (200, 100, 100, 255 );
        g_renderer->Blit( explosion, m_longitude.DoubleValue() - explosionSize/4,
						  m_latitude.DoubleValue() - explosionSize/4, explosionSize/2, explosionSize/2, fire);
    }


}

Fixed Saucer::GetActionRange()
{
    return 0;
}

void Saucer::Retaliate( WorldObjectReference const & attackerId )
{
    // do nothing for now.
}

void Saucer::GetNewTarget()
{
    SetState(0);
    int count = 0;
    int randomCity = syncrand() % g_app->GetWorld()->m_cities.Size();
    City *city = g_app->GetWorld()->m_cities[randomCity];
    while( city->m_population == 0 &&
           count < 20 )
    {
        randomCity = syncrand() % g_app->GetWorld()->m_cities.Size();
        city = g_app->GetWorld()->m_cities[randomCity];
        count++;
    }
    if( city->m_population > 0 )
    {
        SetWaypoint( city->m_longitude, city->m_latitude );
        m_targetObjectId = randomCity;
        m_explosionSize = 0;
    }
    else
    {
        m_leavingWorld = true;
    }
}
