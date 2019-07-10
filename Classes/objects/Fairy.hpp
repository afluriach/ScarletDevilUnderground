//
//  Fairy.hpp
//  Koumachika
//
//  Created by Toni on 11/30/18.
//
//

#ifndef Fairy_hpp
#define Fairy_hpp

#include "Collectibles.hpp"
#include "Enemy.hpp"

class RedFairy : public Enemy
{
public:
	static const string baseAttributes;
	static const string properName;

	static const DamageInfo explosionEffect;
	static const SpaceFloat explosionRadius;
	static const float bombCost;

	RedFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	ParamsGeneratorType getBombs();

	virtual inline SpaceFloat getRadarRadius() const { return 7.5; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline string getSprite() const { return "redFairy"; }

	virtual inline string initStateMachine() { return "red_fairy"; }
	virtual void onZeroHP();
};

class ZombieFairy : public Enemy
{
public:
	static const string baseAttributes;
	static const string properName;

	ZombieFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual void init();

	inline virtual DamageInfo touchEffect() const {
		return DamageInfo{ 20.0f, Attribute::darknessDamage, DamageType::touch };
	}

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline string getSprite() const { return "zombieFairy"; }

	virtual inline string initStateMachine() { return "zombie_fairy"; }
};

class Fairy2 : public Enemy
{
public:

	enum class ai_state {
		normal,
		flee,
		fleeWithSupport,
		supportOffered,
		supporting
	};

	enum class ai_priority {
		engage,
		support,
		flee,
	};

	static const string baseAttributes;
	static const float lowHealthRatio;

	Fairy2(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	inline string getSprite() const { return "redFairy2"; }

	virtual inline string initStateMachine() { return "fairy2"; }
	void addFleeThread();
	void addSupportThread(object_ref<Fairy2> other);
	void removeSupportThread();

	virtual void update();

	void requestSupport();
	object_ref<Fairy2> requestHandler(object_ref<Fairy2> other);
	void responseHandler(object_ref<Fairy2> supporting);
	void acknowledgeHandaler(object_ref<Fairy2> supportTarget);
protected:
	ai_state crntState = ai_state::normal;
	unsigned int supportThread = 0;
	object_ref<Fairy2> supportingAgent;
};

#endif /* Fairy_hpp */
