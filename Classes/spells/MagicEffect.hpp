//
//  MagicEffect.hpp
//  Koumachika
//
//  Created by Toni on 11/28/18.
//
//

#ifndef MagicEffect_hpp
#define MagicEffect_hpp

make_static_member_detector(canApply)
make_static_member_detector(flags)

enum class effect_flags
{
	none = 0x0,

	//Effect is applied on attach to object, and is not stored in system.
	immediate = 0x1,
	//Effect persists for some amount of time, either a positive time length, or indefinite.
	durable = 0x2,

	//The effect uses update.
	active = 0x4,
	//The effect can only be attached to Agents, typically for affecting Attributes.
	agent = 0x8,
};

struct effect_params
{
	GObject* target;
	effect_flags flags;
	const MagicEffectDescriptor* desc;
	effect_attributes attr;
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

	MagicEffect(effect_params params);
	inline virtual ~MagicEffect() {}

	GSpace* getSpace() const;

	bool isImmediate() const;
	bool isDurable() const;
	bool isActive() const;
	bool isAgentEffect() const;

	virtual void init() = 0;
	inline virtual void update() {}
	inline virtual void end() {}

	inline const MagicEffectDescriptor* getDesc() const { return desc; }
	inline GObject* getTarget() const { return target; }
	inline Agent* getAgent() const { return agent; }
	inline float getLength() const { return length; }
	inline float getMagnitude() const { return magnitude; }
	inline state getState() const { return crntState; }
	inline effect_flags getFlags() const { return _flags; }

    void runInit();
    void runUpdate();
    void runEnd();
	void remove();

	const MagicEffectDescriptor* desc;
	GObject* target;
	Agent* agent = nullptr;
	float length, magnitude;
    float t = 0.0f;
	state crntState;
	DamageType damageType;
	effect_flags _flags;

	int _refcount = 0;
};

class AgentEffect : public MagicEffect
{
public:
	AgentEffect(effect_params params);
	virtual ~AgentEffect();

	Agent *const agent;
};

class MagicEffectDescriptor
{
public:
	inline MagicEffectDescriptor(string typeName, effect_flags flags) :
		typeName(typeName),
		flags(flags)
	{}
	virtual inline ~MagicEffectDescriptor() {}

	virtual bool canApply(GObject* target, effect_attributes attr) const = 0;
	virtual local_shared_ptr<MagicEffect> generate(effect_params params) const = 0;

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

	inline virtual bool canApply(GObject* target, effect_attributes attr) const {
		if constexpr (has_canApply<T>::value) {
			return T::canApply(target, attr, get<Params>(_params)...);
		}
		else {
			return true;
		}
	}

	inline virtual local_shared_ptr<MagicEffect> generate(effect_params params) const {
		return make_local_shared<T>(params, get<Params>(_params)...);
	}
protected:
	tuple<Params...> _params;
};

class ScriptedMagicEffect : public MagicEffect
{
public:
	static effect_flags getFlags(string clsName);

	ScriptedMagicEffect(effect_params params, string clsName);

	virtual void init();
	virtual void update();
	virtual void end();

	inline string getClsName() const { return clsName; }
protected:
	string clsName;
	sol::table obj;
};

#endif /* MagicEffect_hpp */
