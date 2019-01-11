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
constexpr inline bitset<enum_size> make_enum_bitfield(E input)
{
	bitset<enum_size> result;

	result[to_size_t(input)] = true;

	return result;
}

#define enum_increment(cls,lval) lval = static_cast<cls>( static_cast<int>(lval) + 1 )
#define enum_foreach(cls,var_name,begin,end) for(cls var_name=cls::begin; var_name < cls::end; enum_increment(cls, var_name) )

#define enum_bitwise_or(cls,a,b) static_cast<cls>(static_cast<unsigned int>(cls::a) | static_cast<unsigned int>(cls::b))
#define enum_bitwise_or3(cls,a,b,c) static_cast<cls>(static_cast<unsigned int>(cls::a) | static_cast<unsigned int>(cls::b) | static_cast<unsigned int>(cls::c))

#define bitwise_and(cls,a,b) static_cast<cls>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b))

#endif enum_h
