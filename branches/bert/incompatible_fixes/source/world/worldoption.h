
#ifndef _included_worldoption_h
#define _included_worldoption_h

#include "lib/tosser/btree.h"

class TextReader;

class WorldOptionBase
{
public:
    WorldOptionBase( char const * name );
    virtual ~WorldOptionBase();
    
    static void LoadAll();

    virtual void Reset() = 0;
protected:
    char const * m_name;
    
    static void Load( TextReader * reader );
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

template<> WorldOption<char const *>::~WorldOption();
template<> void WorldOption<char const *>::Reset();
template<> WorldOption<char const *>::WorldOption( char const * name, char const * const & def );

typedef WorldOption<char const *> WorldOptionString;

class TempName
{
    enum { Len = 100 };
    char m_name[Len];

public:
    operator char const *() const;
    TempName( int type, char const * stem );
    TempName( int type1, int type2, char const * stem );
};

#endif

