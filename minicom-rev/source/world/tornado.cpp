#include "lib/universal_include.h"

#include <math.h>

#include "lib/resource/resource.h"
#include "lib/resource/image.h"
#include "lib/render/renderer.h"
#include "lib/math/vector3.h"
#include "lib/math/random_number.h"

#include "app/app.h"
#include "app/globals.h"

#include "renderer/map_renderer.h"

#include "interface/interface.h"

#include "world/world.h"
#include "world/tornado.h"
#include "world/team.h"
#include "world/nuke.h"

static MovingUnitSettings s_tornadoSettings( WorldObject::TypeTornado, 1, 10, 4, 600 );
static StateSettings s_tornadoTornado( WorldObject::TypeTornado, "", 0, 0, 5, Fixed::MAX, false );


Tornado::Tornado()
:   MovingObject(),
    m_targetObjectId(-1)
{
    Setup( TypeTornado, s_tornadoSettings );

    strcpy( bmpImageFilename, "graphics/tornado.bmp" );

    // m_radarRange = 0;
    m_selectable = true;
    m_maxHistorySize = 10;
	m_angle = 0.0f;
    m_movementType = MovementTypeAir;
	AddState( "WOOSH", s_tornadoTornado );

    InitialiseTimers();
}


void Tornado::Action( WorldObjectReference const & targetObjectId, float longitude, float latitude )
{
    
}

bool Tornado::Update()
{   
	for(int i = 0; i < g_app->GetWorld()->m_objects.Size(); ++i)
	{
		if( g_app->GetWorld()->m_objects.ValidIndex(i) )
		{
			if( g_app->GetWorld()->m_objects[i]->m_type == WorldObject::TypeNuke )
			{
				Nuke *nuke = (Nuke *)g_app->GetWorld()->m_objects[i];
				Fixed distance = g_app->GetWorld()->GetDistance( m_longitude, m_latitude, nuke->m_longitude, nuke->m_latitude);
				if( distance <= GetActionRange() && -1 == m_deflectedNukeIds.FindData(nuke->m_objectId) )
				{
					Fixed targetLongitude = syncsfrand(360);
					Fixed targetLatitude = syncsfrand(180);  
                    nuke->m_targetLocked = false;
					nuke->SetWaypoint(targetLongitude, targetLatitude);
                    m_deflectedNukeIds.PutDataAtStart(nuke->m_objectId);
				}
			}
			else if( g_app->GetWorld()->m_objects[i]->m_type == WorldObject::TypeTornado )
			{
				if( m_size < 40 )
				{
					Tornado *tornado = (Tornado *)g_app->GetWorld()->m_objects[i];
					Fixed distance = g_app->GetWorld()->GetDistance( m_longitude, m_latitude, tornado->m_longitude, tornado->m_latitude);
					if( distance <= GetActionRange() )
					{
						if( tornado->m_objectId != m_objectId )
						{
							if( tornado->GetSize() > m_size )
							{
								tornado->SetSize( tornado->GetSize() + Fixed::Hundredths(20) );
								m_size -= Fixed::Hundredths(20);
							}
						}
					}
				}
			}
		}
	}

    bool arrived = MoveToWaypoint();
    if( arrived )
    {
        GetNewTarget();
    }

	m_size	-= Fixed::FromDouble(0.001f);

    bool amIDead = MovingObject::Update();  
	
	if( m_size < Fixed::Hundredths(50) ) amIDead = true;

    return amIDead;
}

Fixed Tornado::GetActionRange()
{
    return m_size / Fixed::FromDouble(1.5f);
}


void Tornado::Render( RenderInfo & renderInfo )
{
    renderInfo.FillPosition(this);

    Colour colour       = COLOUR_SPECIALOBJECTS;         

    m_angle += 0.05f;

    Image *bmpImage = g_resource->GetImage( bmpImageFilename );
    g_renderer->Blit( bmpImage, renderInfo.m_position.x + renderInfo.m_velocity.x * 10,
					  renderInfo.m_position.y + renderInfo.m_velocity.y * 10, m_size.DoubleValue()/2, m_size.DoubleValue()/2,
					  colour, m_angle );
}

Fixed Tornado::GetSize()
{
	return m_size;
}

void Tornado::SetSize( Fixed size )
{
	m_size = size;
}

void Tornado::GetNewTarget()
{
    Fixed targetLongitude = syncsfrand(360);
	Fixed targetLatitude = syncsfrand(180);  
    SetWaypoint( targetLongitude, targetLatitude );
}
