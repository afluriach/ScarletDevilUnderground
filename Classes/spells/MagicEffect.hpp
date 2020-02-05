//
//  MagicEffect.hpp
//  Koumachika
//
//  Created by Toni on 11/28/18.
//
//

#ifndef MagicEffect_hpp
#define MagicEffect_hpp

class RadarSensor;

make_static_member_detector(canApply)
make_static_member_detector(flags)

enum class effect_flags
{
	none = 0x0,

	//Effect is simply applied on attach to object, and doesn't actually need to be stored.
	immediate = 0x1,
	//Effect does not have time length, parameter will be ignored.
	indefinite = 0x2,
	//Effect should have a physically real (positive) time length.
	timed = 0x4,

	//The effect uses update.
	active = 0x8,
};

struct effect_params
{
	GObject* target;
	unsigned int id;
	effect_flags flags;
	local_shared_ptr<MagicEffectDescriptor> desc;
};

class MagicEffect
{
public:
	enum class state
	{
		created = 1,
		active,
		ending,
		expired,
	};

	MagicEffect(effect_params params, float magnitude, float length);

	GSpace* getSpace() const;

	bool isImmediate() const;
	bool isTimed() const;
	bool isActive() const;

	inline virtual ~MagicEffect() {}

	virtual void init() = 0;
	inline virtual void update() {}
	inline virtual void end() {}

	//remove this - queues this magic effect for removal, can be called within an update
	void remove();

	local_shared_ptr<MagicEffectDescriptor> desc;
	GObject* target;
	float length, magnitude;
	state crntState;
	unsigned int id;
	effect_flags _flags;
};

class MagicEffectDescriptor
{
public:
	inline MagicEffectDescriptor(string typeName, effect_flags flags) :
		typeName(typeName),
		flags(flags)
	{}
	virtual inline ~MagicEffectDescriptor() {}

	virtual bool canApply(GObject* target, float magnitude, float length) const = 0;
	virtual local_shared_ptr<MagicEffect> generate(effect_params params, float magnitude, float length) const = 0;

	inline string getTypeName() const { return typeName; }
	inline effect_flags getFlags() const { return flags; }
protected:
	string typeName;
	effect_flags flags;
};

template<class T, typename... Params>
class MagicEffectDescImpl : public MagicEffectDescriptor
{
public:
	static effect_flags getFlags(Params... params)
	{
		if constexpr (has_flags<T>::value) {
			return T::flags;
		}
		else {
			return T::getFlags(params...);
		}
	}

	inline MagicEffectDescImpl(string typeName, Params... params) :
		MagicEffectDescriptor(typeName, getFlags(params...)),
		_params(forward_as_tuple(params...))
	{
	}

	inline virtual bool canApply(GObject* target, float magnitude, float length) const {
		if constexpr (has_canApply<T>::value) {
			return T::canApply(target, magnitude, length, get<Params>(_params)...);
		}
		else {
			return true;
		}
	}

	inline virtual local_shared_ptr<MagicEffect> generate(effect_params params, float magnitude, float length) const {
		return make_local_shared<T>(params, magnitude, length, get<Params>(_params)...);
	}
protected:
	tuple<Params...> _params;
};

class ScriptedMagicEffect : public MagicEffect
{
public:
	static effect_flags getFlags(string clsName);

	ScriptedMagicEffect(effect_params params, float magnitude, float length, string clsName);

	virtual void init();
	virtual void update();
	virtual void end();

	inline string getClsName() const { return clsName; }
protected:
	string clsName;
	sol::table obj;
};

class RadiusEffect : public MagicEffect
{
public:
	RadiusEffect(effect_params params, SpaceFloat radius, GType type);

	virtual void init();
	virtual void update();
	virtual void end();

	//interface for sensor
	void onContact(GObject* obj);
	void onEndContact(GObject* obj);

	virtual void onHit(GObject* target) = 0;
protected:
	unordered_set<GObject*> contacts;
	RadarSensor* sensor = nullptr;

	SpaceFloat radius;
	GType type;
};

class DamageRadiusEffect : public RadiusEffect
{
public:
	DamageRadiusEffect(effect_params params, DamageInfo damage, SpaceFloat radius, GType type);

	virtual void onHit(GObject* target);
protected:
	DamageInfo damage;
};

#endif /* MagicEffect_hpp */
