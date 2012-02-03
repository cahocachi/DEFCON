
#ifndef _included_worldoption_h
#define _included_worldoption_h

#include "lib/tosser/btree.h"

class WorldOptionBase
{
public:
    WorldOptionBase( char const * name );
    virtual ~WorldOptionBase();
    
    static void LoadAll();

    virtual void Reset() = 0;
protected:
    char const * m_name;
    
    static void LoadFile( char const * fullPath );
private:
    static BTree< WorldOptionBase * > & GetWorldOptions();
    
    virtual bool Set( char const * value ) = 0;
};

template< class T >
class WorldOption: public WorldOptionBase
{
public:
    WorldOption( char const * name, T const & def )
    : WorldOptionBase( name ), m_data( def ), m_default( def )
    {}

    virtual ~WorldOption(){}

    T const & Get(){ return m_data; }
    operator const T &() { return m_data; }
private:
    virtual void Reset(){ m_data = m_default; }
    virtual bool Set( char const * value );

    bool SetDefault( char const * value );

    T m_data;
    T m_default;
};

#endif

