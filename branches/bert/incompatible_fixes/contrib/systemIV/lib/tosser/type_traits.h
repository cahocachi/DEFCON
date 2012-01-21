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
// could probably be improved a lot to do better selection; 0 should only be used
// for builtin types.
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
public:
    template<class T> T static const & DefaultValue()
    {
        return DefaultValueHelper<T>( Selector< (sizeof(T) > 8) >::Get() );
    }
};

#endif
