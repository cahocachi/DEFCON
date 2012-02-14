#include "lib/universal_include.h"
#include "lib/debug_utils.h"

#include "lib/filesys/text_stream_readers.h"
#include "lib/filesys/file_system.h"
#include "lib/filesys/filesys_utils.h"
#include "lib/string_utils.h"

#include "world/worldoption.h"

#include "lib/math/fixed.h"
#include <sstream>

WorldOptionBase::WorldOptionBase( char const * name )
: m_name( newStr( name ) )
{
    AppDebugAssert( !GetWorldOptions().GetData( name ) );

    GetWorldOptions().PutData( m_name, this );
}

WorldOptionBase::~WorldOptionBase()
{
    GetWorldOptions().RemoveData( m_name );

    delete[] m_name;
}

static char const * s_optionPath = "data/worldoptions.txt";

static void ResetAll( BTree< WorldOptionBase * > * tree )
{
    if( tree )
    {
        if( tree->data )
        {
            tree->data->Reset();
        }
        ResetAll( tree->Left() );
        ResetAll( tree->Right() );
    }
}

void WorldOptionBase::LoadAll()
{
    ResetAll( &GetWorldOptions() );

    if( !g_fileSystem )
    {
        return;
    }

    // go through search path in reverse order, later changes overide earlier ones
    Load( g_fileSystem->GetTextReaderDefault( s_optionPath )  );
    for( int i = g_fileSystem->m_searchPath.Size()-1; i >= 0 ; --i )
    {
        char * fullPath = ConcatPaths( g_fileSystem->m_searchPath[i], s_optionPath, NULL );
        if( !DoesFileExist( fullPath ) )
        {
            Load( new TextFileReader( fullPath ) );
        }
        delete[] fullPath;
    }
}

void WorldOptionBase::Load( TextReader * in )
{
    if( in && in->IsOpen() )
    {
        while( in->ReadLine() )
        {
            if( !in->TokenAvailable() ) continue;
            char * param = in->GetNextToken();
            if( !in->TokenAvailable() ) continue;
            char * value = in->GetRestOfLine();

            // discard newlines (yes, this modifies the string in the reader itself; that's
            // fine, we no longer need it.)
            int len = strlen(value);
            while( len > 0 && ( value[len-1] == '\n' || value[len-1] == '\r' ) )
            {
                value[len-1] = 0;
                --len;
            }

            WorldOptionBase * opt = GetWorldOptions().GetData( param );
            if( opt  )
            {
                if( !opt->Set( value ) )
                {
                    AppDebugOut( "Reading worldoptions from %s:%d: Parsing value '%s' failed.\n", in->GetFilename(), in->m_lineNum, value );
                }
            }
            else
            {
                AppDebugOut( "Reading worldoptions from %s:%d: Option '%s' not found.\n", in->GetFilename(), in->m_lineNum, param );
            }
        }
    }
    delete in;
}

BTree< WorldOptionBase * > & WorldOptionBase::GetWorldOptions()
{
    static BTree< WorldOptionBase * > worldOptions;
    return worldOptions;
}
    

template< class T >
bool WorldOption<T>::SetDefault( char const * value )
{
    std::istringstream s( value );
    s >> m_data;
    return s.good();
}

template<>
bool WorldOption<int>::Set( char const * value )
{
    char * end = NULL;
    int parsed = strtol( value, &end, 10 );
    bool ret = end && ( isspace(*end) ||  *end == 0 );
    if( ret )
    {
#ifdef _DEBUG
        if( m_data != parsed )
        {
            AppDebugOut( "Readig worldoptions: changed %s from %d to %d.\n", m_name, m_data, parsed );
        }
#endif
        m_data = parsed;
    }

    return ret;
}

template<>
bool WorldOption<Fixed>::Set( char const * value )
{
    Fixed parsed;
    bool ret = false;
    if( 0 == strncmp( "MAX", value, 3 ) )
    {
        parsed = Fixed::MAX;
        ret = true;
    }
    else
    {
        char * end = NULL;
        parsed = Fixed::FromDouble( strtod( value, &end ) );
        ret = end && ( isspace(*end) ||  *end == 0 );
    }
    if( ret )
    {
#ifdef _DEBUG
        if( m_data != parsed )
        {
            AppDebugOut( "Readig worldoptions: changed %s from %fL to %fL.\n", m_name, m_data.DoubleValue(), parsed.DoubleValue() );
        }
#endif
        m_data = parsed;
    }

    return ret;
}

template<> WorldOption<char const *>::~WorldOption()
{
    delete[] m_data;
    delete[] m_default;
}

template<> void WorldOption<char const *>::Reset()
{
    delete[] m_data;
    m_data = newStr(m_default);
}

template<> WorldOption<char const *>::WorldOption( char const * name, char const * const & def )
: WorldOptionBase( name ), m_data( newStr(def) ), m_default( newStr(def) )
{
}

template<>
bool WorldOption<char const *>::Set( char const * value )
{
    delete[] m_data;
    m_data = newStr( value );
    return true;
}

#ifdef _DEBUG
WorldOption< int > g_testInt( "TestInt", 1 );
WorldOption< Fixed > g_testFixed( "TestFixed", 0 );
WorldOptionString g_testString( "TestString", "Bla" );
#endif
