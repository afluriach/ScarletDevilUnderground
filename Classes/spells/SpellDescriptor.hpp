//
//  SpellDescriptor.hpp
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef SpellDescriptor_hpp
#define SpellDescriptor_hpp

#include "spell_types.hpp"

make_static_member_detector(cost)
make_static_member_detector(icon)

class SpellDesc
{
public:
	inline SpellDesc(spell_params params) : params(params) {}
	virtual inline ~SpellDesc() {}

	inline string getName() const { return params.name; }
	inline string getDescription() const { return params.description; }
	inline string getIcon() const { return params.icon; }
	inline spell_cost getCost() const { return params.cost; }

	virtual local_shared_ptr<Spell> generate(GObject* caster, unsigned int id) const = 0;

	spell_params params;
};

class ScriptedSpellDescriptor : public SpellDesc
{
public:
	static sol::table getClsObject(const string& clsName);
	static spell_params getParams(const string& clsName);

	ScriptedSpellDescriptor(string clsName);
	virtual inline ~ScriptedSpellDescriptor() {}

	virtual local_shared_ptr<Spell> generate(GObject* caster, unsigned int id) const;
protected:
	string clsName;
};

//Use CRTP to get static constants from Spell class.
template<class T, typename... ClsParams>
class SpellDescImpl : public SpellDesc
{
public:
	inline SpellDescImpl(spell_params params, ClsParams... _params) : 
		SpellDesc(params),
		clsparams(forward_as_tuple(_params)...)
	{}
	virtual inline ~SpellDescImpl() {}

	virtual inline local_shared_ptr<Spell> generate(GObject* caster, unsigned int id) const
	{
		return make_local_shared<T>(caster, this, id, get<ClsParams>(clsparams)...);
	}

	tuple<ClsParams...> clsparams;
};

#endif /* SpellDescriptor_hpp */
