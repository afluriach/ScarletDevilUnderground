//
//  lua_struct.cpp
//  Koumachika
//
//  Created by Toni on 2/29/20.
//
//

#include "Prefix.h"

#include "LuaAPI.hpp"

namespace Lua{
	
struct hashset_gobject_ref
{
	unordered_set<gobject_ref> _set;

	inline void insert(const gobject_ref& ref)
	{
		_set.insert(ref);
	}

	inline void erase(const gobject_ref& ref)
	{
		_set.erase(ref);
	}

	inline bool contains(const gobject_ref& ref) const
	{
		return _set.find(ref) != _set.end();
	}

	inline void clear()
	{
		_set.clear();
	}

	inline size_t size() const
	{
		return _set.size();
	}

	inline void for_each(function<void(const gobject_ref&)> f) const
	{
		for (auto const& ref : _set) {
			f(ref);
		}
	}

	inline sol::table getArray() const
	{
		int idx = 1;
		sol::table t = GSpace::scriptVM->_state.create_table();

		for (auto const& ref : _set) {
			t[idx++] = ref;
		}

		return t;
	}
};

void Inst::addStructures()
{
	auto hashset = _state.new_usertype<hashset_gobject_ref>(
		"hashset_gobject_ref"
	);
	hashset["insert"] = &hashset_gobject_ref::insert;
	hashset["contains"] = &hashset_gobject_ref::contains;
	hashset["erase"] = &hashset_gobject_ref::erase;
	hashset["clear"] = &hashset_gobject_ref::clear;
	hashset["size"] = &hashset_gobject_ref::size;
	hashset["for_each"] = &hashset_gobject_ref::for_each;
	hashset["getArray"] = &hashset_gobject_ref::getArray;
}

}
