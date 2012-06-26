#ifndef MLPLUS_TYPELIST_H 
#define MLPLUS_TYPELIST_H 

namespace mlplus
{
class NullType {};

template <class T, class U>
struct Typelist
{
    typedef T Head;
    typedef U Tail;
};

template
<
    typename T1  = NullType, typename T2  = NullType, typename T3  = NullType,
    typename T4  = NullType, typename T5  = NullType, typename T6  = NullType,
    typename T7  = NullType, typename T8  = NullType, typename T9  = NullType,
    typename T10 = NullType, typename T11 = NullType, typename T12 = NullType,
    typename T13 = NullType, typename T14 = NullType, typename T15 = NullType,
    typename T16 = NullType, typename T17 = NullType, typename T18 = NullType
>
struct MakeTypelist
{
private:
    typedef typename MakeTypelist
    <
       T2 , T3 , T4 ,
       T5 , T6 , T7 ,
       T8 , T9 , T10,
       T11, T12, T13,
       T14, T15, T16,
       T17, T18
    >::Result TailResult;
public:
    typedef Typelist<T1, TailResult> Result;
};

template<>
struct MakeTypelist<>
{
    typedef NullType Result;
};


template <class TList, unsigned int index> struct TypeAt;

template <class Head, class Tail>
struct TypeAt<Typelist<Head, Tail>, 0>
{
    typedef Head Result;
};

template <class Head, class Tail, unsigned int i>
struct TypeAt<Typelist<Head, Tail>, i>
{
    typedef typename TypeAt < Tail, i - 1 >::Result Result;
};


template <class TList, class T> struct IndexOf;
template <class T>
struct IndexOf<NullType, T>
{
    enum { value = -1 };
};

template <class T, class Tail>
struct IndexOf<Typelist<T, Tail>, T>
{
    enum { value = 0 };
};

template <class Head, class Tail, class T>
struct IndexOf<Typelist<Head, Tail>, T>
{
private:
    enum { temp = IndexOf<Tail, T>::value };
public:
    enum { value = (temp == -1 ? -1 : 1 + temp) };
};


template <class TList, class T> struct Append;

template <> struct Append<NullType, NullType>
{
    typedef NullType Result;
};

template <class T> struct Append<NullType, T>
{
    typedef Typelist<T, NullType> Result;
};

template <class Head, class Tail>
struct Append<NullType, Typelist<Head, Tail> >
{
    typedef Typelist<Head, Tail> Result;
};

template <class Head, class Tail, class T>
struct Append<Typelist<Head, Tail>, T>
{
    typedef Typelist < Head,
            typename Append<Tail, T>::Result >
            Result;
};



}   // namespace mlplus

