//
//  MagicEffect.hpp
//  Koumachika
//
//  Created by Toni on 11/28/18.
//
//

#ifndef MagicEffect_hpp
#define MagicEffect_hpp

#include "object_ref.hpp"

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

	inline MagicEffect(GObject* agent) : MagicEffect(agent, 0.0f) {}
	MagicEffect(GObject* agent, float magnitude);

	GSpace* getSpace() const;

	inline virtual ~MagicEffect() {}

	virtual void init() = 0;
	virtual void update() = 0;
	virtual void end() = 0;

	GObject* agent;
	float magnitude;
	state crntState;
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
