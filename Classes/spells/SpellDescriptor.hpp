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

	virtual shared_ptr<Spell> generate(GObject* caster) = 0;
	virtual SpellGeneratorType getGenerator() = 0;
};

//Use CRTP to get static constants from Spell class.
template<class T>
class SpellDescImpl : public SpellDesc
{
public:
	virtual inline string getName() const { return T::name; }
	virtual inline string getDescription() const { return T::description; }

	virtual inline shared_ptr<Spell> generate(GObject* caster)
	{
		return make_shared<T>(caster);
	}

	virtual inline SpellGeneratorType getGenerator()
	{
		return make_spell_generator<T>();
	}
};

#endif /* SpellDescriptor_hpp */
