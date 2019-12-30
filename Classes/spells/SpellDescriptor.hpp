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
	inline SpellDesc() {}
	virtual inline ~SpellDesc() {}

	virtual string getName() const = 0;
	virtual string getDescription() const = 0;
	virtual string getIcon() const = 0;

	virtual spell_cost getCost() const = 0;

	virtual Spell* generate(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id) = 0;
};

class ScriptedSpellDescriptor : public SpellDesc
{
public:
	static sol::table getClsObject(const string& clsName);

	ScriptedSpellDescriptor(string clsName);
	virtual inline ~ScriptedSpellDescriptor() {}

	virtual string getName() const;
	virtual string getDescription() const;
	virtual string getIcon() const;
	virtual spell_cost getCost() const;

	virtual Spell* generate(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id);
protected:
	string clsName;
};

//Use CRTP to get static constants from Spell class.
template<class T>
class SpellDescImpl : public SpellDesc
{
public:
	inline SpellDescImpl() {}
	virtual inline ~SpellDescImpl() {}

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

	virtual inline Spell* generate(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id)
	{
		return new T(caster, desc, id);
	}
};

#endif /* SpellDescriptor_hpp */
