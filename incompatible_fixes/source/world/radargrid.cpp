#include "lib/universal_include.h"
#include "lib/debug_utils.h"
#include "lib/resource/bitmap.h"
#include "lib/resource/image.h"
#include "lib/profiler.h"

#include "world/radargrid.h"
#include "world/world.h"

#include "app/app.h"
#include "app/globals.h"

/*
// crude function to profile whether reads or writes are more frequent;
// result: twice as many writes.
static void MoreReadsOrMoreWrites(int i)
{
    static int c[2];
    c[i]++;

    if( 0 == c[i] % 10000 )
    {
        printf("%d %d\n",i,c[i]);
    }
}
*/

RadarGrid::RadarGrid()
: m_radar(NULL), 
  m_resolution(0),
  m_teamCount(0)
{
}

RadarGrid::~RadarGrid()
{
    delete[] m_radar;
}

void RadarGrid::Initialise( int _resolution, int _numTeams )
{
    AppDebugAssert( _resolution >= 1 );

    m_resolution = _resolution;
    m_teamCount  = _numTeams;


    int size = RADARGRID_WIDTH * RADARGRID_HEIGHT * m_resolution * m_resolution * _numTeams;

    delete[] m_radar;
    m_radar = new unsigned char[ size ];;
    memset( m_radar, 0, size );
}


void RadarGrid::GetIndices( Fixed _longitude, Fixed _latitude, int &_x, int &_y )
{
    _longitude += 180;
    _latitude += 200/2;

    _x = ( RADARGRID_WIDTH * m_resolution * (_longitude/360) ).IntValue();
    _y = ( RADARGRID_HEIGHT * m_resolution * (_latitude/200) ).IntValue();

    _x = max( _x, 0 );
    _y = max( _y, 0 );
    _x = min( _x, RADARGRID_WIDTH * m_resolution - 1 );
    _y = min( _y, RADARGRID_HEIGHT * m_resolution - 1 );
}


void RadarGrid::GetIndices( Fixed _longitude, Fixed _latitude, Fixed _radius,
                            int &_x, int &_y, int &_w, int &_h )
{
    _longitude += 180;
    _latitude += 200/2;

    int totalW = RADARGRID_WIDTH * m_resolution;
    int totalH = RADARGRID_HEIGHT * m_resolution;

	_x = ( totalW * (_longitude-_radius)/360 ).IntValue();
	_y = ( totalH * (_latitude-_radius)/200 ).IntValue();
	_w = ( totalW * (_radius*2)/360 ).IntValue();
	_h = ( totalH * (_radius*2)/200 ).IntValue();

    //_x = max( _x, 0 );
    _y = max( _y, 0 );
    //_x = min( _x, totalW - 1 );
    _y = min( _y, totalH - 1 );

    //if( _x + _w > totalW - 1 ) _w = totalW - _x - 1;
    if( _y + _h > totalH - 1 ) _h = totalH - _y - 1;
}


void RadarGrid::GetIndicesRadar( int &_x, int &_y )
{
    int totalW = RADARGRID_WIDTH * m_resolution;

    if( _x < 0 )
    {
        _x += totalW;
    }
    else if( _x >= totalW )
    {
        _x -= totalW;
    }

    _x = max( _x, 0 );
    _x = min( _x, totalW - 1 );
}


void RadarGrid::GetWorldLocation( int _indexX, int _indexY,         
                                  Fixed &_longitude, Fixed &_latitude ) const
{
    Fixed totalW = RADARGRID_WIDTH * m_resolution;
    Fixed totalH = RADARGRID_HEIGHT * m_resolution;

    Fixed cellSizeW = 360 / totalW;
    Fixed cellSizeH = 200 / totalH;
    
    _longitude = 360 * _indexX/totalW;
    _latitude = 200 * _indexY/totalH;

    _longitude += cellSizeW/2;
    _latitude += cellSizeH/2;

    _longitude -= 360/2;
    _latitude -= 200/2;
}

inline bool RadarGrid::IsInside( int x, int y, Fixed const & radiusSquared, Vector2<Fixed> const & centre ) const
{
    Fixed thisLongitude;
    Fixed thisLatitude;
    GetWorldLocation( x, y, thisLongitude, thisLatitude );

    Fixed distanceSquared = ( centre - Vector2<Fixed>(thisLongitude, thisLatitude) ).MagSquared();
    return( distanceSquared < radiusSquared );
}

void RadarGrid::ModifyCoverage( Fixed _longitude, Fixed _latitude, Fixed _radius, int _teamId, bool addCoverage )
{
    if( _teamId == -1 ) return;

    //
    // Round longitude and latitude to grid square centres

    Fixed radiusSquared = _radius * _radius;

    int centreX, centreY;
    GetIndices( _longitude, _latitude, centreX, centreY );
    GetWorldLocation( centreX, centreY, _longitude, _latitude );

    int x0, y0, w, h;
    GetIndices( _longitude, _latitude, _radius, x0, y0, w, h );

    int singleSize = RADARGRID_WIDTH * RADARGRID_HEIGHT * m_resolution * m_resolution;
    unsigned char * array = m_radar + _teamId * m_resolution * m_resolution * singleSize;
    char increment = addCoverage ? +1 : -1;

    unsigned int totalWidth = RADARGRID_WIDTH * m_resolution;
    unsigned int totalWidthAdd = 3*totalWidth;

    // borders of region to fill in x direction
    int xMid = x0+(w>>1);
    int xMin = xMid;
    int xMax = xMid-1;

    Vector2<Fixed> centre(_longitude, _latitude);

    for( int y = y0; y < y0+h; ++y )
    {
        unsigned char * row = array + y * totalWidth;

        // update fill region
        if( xMin > x0 && IsInside( xMin-1, y, radiusSquared, centre ) )
        {
            do
            {
                xMin--;
            }
            while ( xMin > x0 && IsInside( xMin-1, y, radiusSquared, centre ) );
        }
        else
        {
            while( xMin < xMid && !IsInside( xMin, y, radiusSquared, centre ) )
            {
                xMin++;
            }
        }
        if( xMax < x0+w && IsInside( xMax+1, y, radiusSquared, centre ) )
        {
            do
            {
                xMax++;
            }
            while( xMax < x0+w && IsInside( xMax+1, y, radiusSquared, centre ) );
        }
        else
        {
            while( xMax >= xMid && !IsInside( xMax, y, radiusSquared, centre ) )
            {
                xMax--;
            }
        }

        AppDebugAssert(!IsInside(xMin-1,y,radiusSquared,centre));
        AppDebugAssert(!IsInside(xMax+1,y,radiusSquared,centre));

        for( int x = xMin; x <= xMax; ++x )
        {
            AppDebugAssert(IsInside(x,y,radiusSquared,centre));
            row[ ((x+totalWidthAdd)%totalWidth) ] += increment;
            // MoreReadsOrMoreWrites(0);
        }
    }
}


void RadarGrid::AddCoverage( Fixed _longitude, Fixed _latitude, Fixed _radius, int _teamId )
{
    ModifyCoverage( _longitude, _latitude, _radius, _teamId, true );
}


void RadarGrid::RemoveCoverage ( Fixed _longitude, Fixed _latitude, Fixed _radius, int _teamId )
{
    ModifyCoverage( _longitude, _latitude, _radius, _teamId, false );
}


void RadarGrid::UpdateCoverage ( Fixed _oldLongitude, Fixed _oldLatitude, Fixed _oldRadius,
                                 Fixed _newLongitude, Fixed _newLatitude, Fixed _newRadius, int _teamId )
{
    if( _teamId == -1 ) return;
    bool changeRequired = false;

    if( _oldRadius != _newRadius ) 
    {
        // Radius has changed - so we obviously need to update
        changeRequired = true;
    }

    if( !changeRequired )
    {
        // Have we actually moved cells?
        // If not then nothing will have changed
        int oldX, oldY, newX, newY;
        GetIndices( _oldLongitude, _oldLatitude, oldX, oldY );
        GetIndices( _newLongitude, _newLatitude, newX, newY );
        
        changeRequired = ( oldX != newX ) || ( oldY != newY );
    }

    if( changeRequired )
    {
        RemoveCoverage( _oldLongitude, _oldLatitude, _oldRadius, _teamId );
        AddCoverage( _newLongitude, _newLatitude, _newRadius, _teamId );
    }
}


int RadarGrid::GetCoverage( Fixed _longitude, Fixed _latitude, int _teamId )
{
    if( _teamId == -1 ) return 0;

    int indexX;
    int indexY;
    GetIndices( _longitude, _latitude, indexX, indexY );

    int size = RADARGRID_WIDTH * RADARGRID_HEIGHT * m_resolution * m_resolution;
    return m_radar[_teamId * size + indexY * RADARGRID_WIDTH * m_resolution + indexX ];
}

void RadarGrid::GetMultiCoverage( Fixed _longitude, Fixed _latitude, BoundedArray< int > & coveragePerTeam )
{
    if( coveragePerTeam.Size() != m_teamCount )
    {
        coveragePerTeam.Initialise( m_teamCount );
    }

    int indexX;
    int indexY;
    GetIndices( _longitude, _latitude, indexX, indexY );

    int offset =  indexY * RADARGRID_WIDTH * m_resolution + indexX;
    int size = RADARGRID_WIDTH * RADARGRID_HEIGHT * m_resolution * m_resolution;

    for( int team = 0; team < m_teamCount; team++ )
    {
        coveragePerTeam[team] = m_radar[team * size + offset];
        // MoreReadsOrMoreWrites(1);
    }
}

void RadarGrid::Render()
{    
    if( !m_radar ) return;

    START_PROFILE( "RadarGrid" );
    
    //
    // Render radar to a bitmap

    Bitmap bitmap( RADARGRID_WIDTH*m_resolution,
                   RADARGRID_HEIGHT*m_resolution );

    int teamId = g_app->GetWorld()->m_myTeamId;
    int size = RADARGRID_WIDTH * RADARGRID_HEIGHT * m_resolution * m_resolution;

    for( int y = 0; y < RADARGRID_HEIGHT * m_resolution; ++y )
    {
        int index = y * RADARGRID_WIDTH * m_resolution;

        for( int x = 0; x < RADARGRID_WIDTH * m_resolution; ++x )
        {
            int count = m_radar[teamId * size + index+x] * 15;
            count = min( count, 200 );
            
            Colour colour( count, count, count );

            bitmap.PutPixel( x, y, colour );
        }
    }


    //
    // Convert bitmap to texture

    Image image( &bitmap );
    image.MakeTexture( true, true );


    //
    // Render it to screen
    
    g_renderer->SetBlendMode( Renderer::BlendModeAdditive );
    g_renderer->Blit( &image, -180, 100, 360, -200, White );
    
    END_PROFILE( "RadarGrid" );
}

