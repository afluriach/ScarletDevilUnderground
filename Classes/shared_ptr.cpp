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

void shared_ptr_system::acquire(id_type id)
{
	auto it = refs.find(id);
	if (it == refs.end()) {
		log("local_shared_ptr acquire: unknown id %u", id);
		return;
	}

	auto& count = it->second;
	count += 1;
}

shared_ptr_system::id_type shared_ptr_system::create()
{
	id_type id = nextID++;

	refs.insert_or_assign(id, 1);

	return id;
}

bool shared_ptr_system::release(id_type id)
{
	auto it = refs.find(id);

	if (it == refs.end()) {
		log("release: %u not present!", id);
		return false;
	}

	bool zero = it->second == 1;

	if (zero) {
		refs.erase(it);
		return true;
	}
	else {
		--it->second;
		return false;
	}
}
