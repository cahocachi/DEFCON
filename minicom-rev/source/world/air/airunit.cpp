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
#include "world/air/airunit.h"
#include "world/blip.h"
#include "world/fleet.h"


AirUnit::AirUnit()
	: MovingObject()
{
	m_movementType = MovementTypeAir;
}

bool AirUnit::IsValidPosition ( Fixed longitude, Fixed latitude )
{
	return true;
}

bool AirUnit::CanLandOn(int _type)
{
	if(_type == WorldObject::TypeAirBase || WorldObject::TypeCarrier)
		return true;
	else
		return false;
}


void AirUnit::Render(RenderInfo & renderInfo)
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

    //
    // Render history
    if( g_preferences->GetInt( PREFS_GRAPHICS_TRAILS ) == 1 )
    {
        RenderHistory( renderInfo );
    }
}

bool AirUnit::SetWaypointOnAction()
{
    return true;
}