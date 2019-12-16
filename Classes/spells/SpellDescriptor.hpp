//
//  SpellDescriptor.hpp
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef SpellDescriptor_hpp
#define SpellDescriptor_hpp

class GObject;
class Spell;

#include "spell_types.hpp"

make_static_member_detector(cost)
make_static_member_detector(icon)

class SpellDesc
{
public:
	virtual string getName() const = 0;
	virtual string getDescription() const = 0;
	virtual string getIcon() const = 0;

	virtual spell_cost getCost() const = 0;

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

	virtual inline string getIcon() const {
		if constexpr(has_icon<T>::value)
			return T::icon;
		else
			return "";
	}

	virtual inline spell_cost getCost() const {
		if constexpr(has_cost<T>::value)
			return T::cost;
		else
			return spell_cost{};
	}

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
