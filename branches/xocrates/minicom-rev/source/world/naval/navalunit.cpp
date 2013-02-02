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
#include "world/naval/navalunit.h"
#include "world/blip.h"
#include "world/fleet.h"


NavalUnit::NavalUnit()
	: MovingObject()
{
	m_movementType = MovementTypeSea;
}

bool NavalUnit::IsValidPosition ( Fixed longitude, Fixed latitude )
{
	if(g_app->GetMapRenderer()->IsValidTerritory( -1, longitude, latitude, true ) ) 
		return true;

	return false;
}