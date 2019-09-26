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

	inline MagicEffect(GObject* agent) : MagicEffect(agent, 0.0f, 0.0f, flag_bits()) {}
	MagicEffect(GObject* agent, float length, float magnitude, flag_bits _flags);

	GSpace* getSpace() const;

	bool isImmediate() const;
	bool isTimed() const;
	bool isActive() const;

	inline virtual ~MagicEffect() {}

	virtual void init() = 0;
	inline virtual void update() {}
	inline virtual void end() {}

	GObject* agent;
	float length, magnitude;
	state crntState;
	flag_bits _flags;
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
	DamageRadiusEffect(GObject* agent, DamageInfo damage, SpaceFloat radius, GType type);

	virtual void onHit(GObject* target);
protected:
	DamageInfo damage;
};

#endif /* MagicEffect_hpp */
