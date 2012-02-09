#include "lib/universal_include.h"
#include "lib/resource/resource.h"
#include "lib/language_table.h"

#include "app/app.h"
#include "app/globals.h"

#include "world/radarstation.h"

static UnitSettings s_radarSettings( WorldObject::TypeRadarStation, 1 );
static StateSettings s_radarState( WorldObject::TypeRadarStation, "", 0, 0, 20, 0, false );

RadarStation::RadarStation()
:   WorldObject()
{
    Setup( TypeRadarStation, s_radarSettings );
    
    strcpy( bmpImageFilename, "graphics/radarstation.bmp" );

    m_radarRange = 30;
    m_selectable = false;  
    
    AddState( LANGUAGEPHRASE("state_radar"), s_radarState );

}
