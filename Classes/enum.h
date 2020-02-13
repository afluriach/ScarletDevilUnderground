//
//  enum.h
//  Koumachika
//
//  Created by Toni on 5/13/18.
//
//

#ifndef enum_h
#define enum_h

template<typename E>
constexpr size_t to_size_t(E e)
{
	return static_cast<size_t>(e);
}

template<typename E, size_t bitset_size>
bool bitset_enum_index(bitset<bitset_size> _b, E e)
{
	return _b[to_size_t(e)];
}

template<typename E, size_t bitset_size>
void bitset_enum_set(bitset<bitset_size>& _b, E e, bool value)
{
	_b[to_size_t(e)] = value;
}

template<typename E, size_t enum_size = to_size_t(E::end)>
inline bitset<enum_size> make_enum_bitfield(E input)
{
	bitset<enum_size> result;

	result[to_size_t(input)] = true;

	return result;
}

template<typename E, typename... Params>
constexpr E enum_bitwise_or_v(E first, Params... params)
{
	unsigned int result = ( to_uint(first) | ... | to_uint(params) );
	return static_cast<E>(result);
}

#define enum_bitfield2(cls, a, b) (make_enum_bitfield(cls::a) | make_enum_bitfield(cls::b))
#define enum_bitfield3(cls, a, b, c) (make_enum_bitfield(cls::a) | make_enum_bitfield(cls::b) | make_enum_bitfield(cls::c))

#define enum_add(cls, e, i) static_cast<cls>(static_cast<int>(cls::e) + i)
#define enum_count(cls, last, first) (static_cast<int>(cls::last) - static_cast<int>(cls::first) + 1)
#define enum_increment(cls,lval) lval = static_cast<cls>( static_cast<int>(lval) + 1 )
#define enum_foreach(cls,var_name,begin,end) for(cls var_name=cls::begin; var_name < cls::end; enum_increment(cls, var_name) )

#define bitwise_or(cls,a,b) static_cast<cls>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b))

#define enum_bitwise_or(cls,a,b) enum_bitwise_or_v<cls>(cls::a, cls::b)
#define enum_bitwise_or3(cls,a,b,c) enum_bitwise_or_v<cls>(cls::a, cls::b, cls::c)
#define enum_bitwise_or4(cls,a,b,c,d) enum_bitwise_or_v<cls>(cls::a, cls::b, cls::c, cls::d)
#define enum_bitwise_or5(cls,a,b,c,d,e) enum_bitwise_or_v<cls>(cls::a, cls::b, cls::c, cls::d, cls::e)
#define enum_bitwise_or6(cls,a,b,c,d,e,f) enum_bitwise_or_v<cls>(cls::a, cls::b, cls::c, cls::d, cls::e, cls::f)

#define bitwise_and_bool(a,b) static_cast<bool>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b))
#define bitwise_and(cls,a,b) static_cast<cls>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b))

#endif enum_h
