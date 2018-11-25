//
//  SpellDescriptor.hpp
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef SpellDescriptor_hpp
#define SpellDescriptor_hpp

#include "Prefix.h"

class GObject;
class Spell;

class SpellDesc
{
public:
	virtual inline string getName() const = 0;
	virtual inline string getDescription() const = 0;

	virtual inline int getInitialCost() const = 0;
	virtual inline int getCostPerSecond() const = 0;

	virtual inline unique_ptr<Spell> generate(GObject* caster) = 0;
	virtual inline unique_ptr<Spell> generate(GObject* caster, const ValueMap& args) = 0;
};

//Use CRTP to get static constants from Spell class.
template<class T>
class SpellDescImpl : public SpellDesc
{
public:
	virtual inline string getName() const { return T::name; }
	virtual inline string getDescription() const { return T::description; }

	virtual inline int getInitialCost() const { return T::initialCost; }
	virtual inline int getCostPerSecond() const { return T::costPerSecond; }

	virtual inline unique_ptr<Spell> generate(GObject* caster)
	{
		return generate(caster, {});
	}

	virtual inline unique_ptr<Spell> generate(GObject* caster, const ValueMap& args)
	{
		return make_unique<T>(caster, args, this);
	}
};

#endif /* SpellDescriptor_hpp */
