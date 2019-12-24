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

	enum class flags
	{
		//Effect is simply applied on attach to object, and doesn't actually need to be stored.
		immediate = 0,
		//Effect does not have time length, parameter will be ignored.
		indefinite,
		//Effect should have a physically real (positive) time length.
		timed,
		
		//The effect uses update.
		active,

		end,
	};

	static constexpr size_t flagCount = to_size_t(flags::end);
	typedef bitset<flagCount> flag_bits;

	static const flag_bits immediate;

	inline MagicEffect(GObject* target) : MagicEffect(target, 0.0f, 0.0f, flag_bits()) {}
	MagicEffect(GObject* target, float length, float magnitude, flag_bits _flags);

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

	GObject* target;
	float length, magnitude;
	state crntState;
	unsigned int id;
	flag_bits _flags;
};

class MagicEffectDescriptor
{
public:
	inline MagicEffectDescriptor(float baseMagnitude) : baseMagnitude(baseMagnitude) {}
	virtual inline ~MagicEffectDescriptor() {}

	virtual bool canApply(GObject* target, float magnitude) const = 0;
	virtual shared_ptr<MagicEffect> generate(GObject* target, float magnitude) const = 0;
protected:
	float baseMagnitude;
};

template<class T, typename... Params>
class MagicEffectDescImpl : public MagicEffectDescriptor
{
public:
	inline MagicEffectDescImpl(float baseMagnitude, Params... params) :
		MagicEffectDescriptor(baseMagnitude),
		_params(forward_as_tuple(params...))
	{
	}

	inline virtual bool canApply(GObject* target, float magnitude) const {
		float actualMag = baseMagnitude * magnitude;
		if constexpr (has_canApply<T>::value) {
			return T::canApply(target, actualMag, get<Params>(_params)...);
		}
		else {
			return true;
		}
	}

	inline virtual shared_ptr<MagicEffect> generate(GObject* target, float magnitude) const {
		float actualMag = baseMagnitude * magnitude;
		return make_shared<T>(target, actualMag, get<Params>(_params)...);
	}
protected:
	tuple<Params...> _params;
};

class ScriptedMagicEffect : public MagicEffect
{
public:
	static flag_bits getFlags(string clsName);

	ScriptedMagicEffect(string clsName, GObject* target);
	ScriptedMagicEffect(string clsName, GObject* target, float length, float magnitude);

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
	RadiusEffect(GObject* agent, SpaceFloat radius, GType type);

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
	DamageRadiusEffect(GObject* target, DamageInfo damage, SpaceFloat radius, GType type);

	virtual void onHit(GObject* target);
protected:
	DamageInfo damage;
};

#endif /* MagicEffect_hpp */
