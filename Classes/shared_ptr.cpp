//
//  shared_ptr.cpp
//  Koumachika
//
//  Created by Toni on 2/14/20.
//
//

#include "Prefix.h"

#include "shared_ptr.hpp"

unique_ptr<shared_ptr_system> shared_ptr_system::inst  = make_unique<shared_ptr_system>();

shared_ptr_system::shared_ptr_system()
{
}

shared_ptr_system* shared_ptr_system::get()
{
	return inst.get();
}

void shared_ptr_system::acquire(shared_object_entry* entry)
{
	++entry->count;
}

shared_ptr_system::shared_object_entry* shared_ptr_system::create()
{
	id_type id = nextID++;
	
	shared_object_entry* entry = local_allocator<shared_object_entry>::allocate();
	new (entry) shared_object_entry(id);

	auto result = refs.insert_or_assign(id, entry);
	return entry;
}

bool shared_ptr_system::release(shared_object_entry* entry)
{
	--entry->count;

	if (entry->count > 0)
		return false;

	auto it = refs.find(entry->id);
	refs.erase(it);

	entry->~shared_object_entry();
	local_allocator<shared_object_entry>::deallocate(entry);
	
	return true;
}
