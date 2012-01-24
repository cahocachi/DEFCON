//===============================================================//
//                        Type Traits                            //
//                                                               //
//                   By Manuel Moos                              //
//                           V0.1                                //
//===============================================================//

#ifndef _included_typetraits_h
#define _included_typetraits_h

// useful little template magic things to help containers and the like
// decide what to do

// selects a suitable default object; 0 for small things, default constructed for larger things
// builtin types get 0.
// 
class DefaultValueSupplier
{
    template< bool > class Selector
    {
    public:
        static Selector const & Get(){ static Selector s; return s; }
    };

    template<class T> T static const & DefaultValueHelper(Selector<false> const &)
    {
        static T ret(0);
        return ret;
    }

    template<class T> T static const & DefaultValueHelper(Selector<true> const &)
    {
        static T ret;
        return ret;
    }

    // catch-all
    template<class T> T static const & DefaultValueByType( void const * )
    {
        return DefaultValueHelper<T>( Selector< (sizeof(T) > 4) >::Get() );
    }

    // pointers
    template<class T> static T * DefaultValueByType( T * const * )
    {
        return 0;
    }

    // builtin types
#define DEFAULT_VALUE_IS_ZERO(t) \
    template<class T> T static const DefaultValueByType( t const * ) \
    { \
        return 0; \
    } \

    DEFAULT_VALUE_IS_ZERO(char)
    DEFAULT_VALUE_IS_ZERO(short)
    DEFAULT_VALUE_IS_ZERO(int)
    DEFAULT_VALUE_IS_ZERO(unsigned char)
    DEFAULT_VALUE_IS_ZERO(unsigned short)
    DEFAULT_VALUE_IS_ZERO(unsigned int)
    DEFAULT_VALUE_IS_ZERO(float)
    DEFAULT_VALUE_IS_ZERO(double)
    DEFAULT_VALUE_IS_ZERO(bool)
public:
    template<class T> T static const & DefaultValue()
    {
        T const * selector = NULL;
        static T def = DefaultValueByType<T>( selector );
        return def;
    }
};

#endif
