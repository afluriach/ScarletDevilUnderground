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

class GhostFairy :
	public Enemy,
	public BaseAttributes<GhostFairy>
{
public:
	static const string baseAttributes;
	static const string properName;

	GhostFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getMass() const { return 10.0; }
	virtual inline GType getType() const { return GType::enemy; }
	virtual inline GType getRadarType() const { return GType::enemySensor; }
	virtual inline SpaceFloat getRadarRadius() const { return 3.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	virtual inline string getSprite() const { return "ghostFairy"; }

	virtual void initStateMachine();
};

class Fairy1 :
public Enemy,
public BaseAttributes<Fairy1>,
public AIPackage<Fairy1>
{
public:
	static const string baseAttributes;
	static const AttributePackageMap attributePackages;
	static const AIPackage<Fairy1>::AIPackageMap aiPackages;

	Fairy1(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
	virtual inline SpaceFloat getMass() const { return 40.0; }

	inline string getSprite() const { return "greenFairy"; }

	void maintain_distance(const ValueMap& args);
	void circle_and_fire(const ValueMap& args);
	void circle_around_point(const ValueMap& args);
	void flock(const ValueMap& args);
};

class BlueFairy :
	public Enemy,
	public AIPackage<BlueFairy>,
	public BaseAttributes<BlueFairy>
{
public:
	static const AIPackage<BlueFairy>::AIPackageMap aiPackages;
	static const string baseAttributes;
	static const string properName;

	BlueFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getMaxSpeed() const { return 2.5; }
	virtual inline SpaceFloat getMaxAcceleration() const { return 6.0; }
	virtual inline SpaceFloat getRadarRadius() const { return 5.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi * 0.5; }
	virtual inline SpaceFloat getMass() const { return 25.0; }

	inline string getSprite() const { return "blueFairy"; }

	void follow_path(const ValueMap& args);
};

class RedFairy : public Enemy, public BaseAttributes<RedFairy>
{
public:
	static const string baseAttributes;
	static const string properName;

	static const DamageInfo explosionEffect;
	static const SpaceFloat explosionRadius;
	static const float bombCost;

	RedFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	BombGeneratorType getBombs();

	virtual inline SpaceFloat getRadarRadius() const { return 7.5; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
	inline SpaceFloat getMass() const { return 25.0; }

	inline string getSprite() const { return "redFairy"; }

	virtual void initStateMachine();
	virtual void onZeroHP();
};

class GreenFairy1 :
	public Enemy,
	public BaseAttributes<GreenFairy1>
{
public:
	static const string baseAttributes;
	static const string properName;

	GreenFairy1(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 4.5; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi; }

	inline SpaceFloat getMass() const { return 15.0; }

	inline string getSprite() const { return "greenFairy"; }

	virtual void initStateMachine();
};

class GreenFairy2 :
	public Enemy,
	public BaseAttributes<GreenFairy2>
{
public:
	static const string baseAttributes;
	static const string properName;

	GreenFairy2(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline SpaceFloat getMass() const { return 25.0; }

	inline string getSprite() const { return "greenFairy2"; }

	virtual void initStateMachine();
	virtual void onRemove();
};


class ZombieFairy :
	public Enemy,
	public BaseAttributes<ZombieFairy>,
	public RegisterInit<ZombieFairy>
{
public:
	static const string baseAttributes;
	static const string properName;

	ZombieFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	void init();

	inline virtual DamageInfo touchEffect() const {
		return DamageInfo{ 20.0f, Attribute::darknessDamage, DamageType::touch };
	}

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline SpaceFloat getMass() const { return 25.0; }

	inline string getSprite() const { return "zombieFairy"; }

	virtual void initStateMachine();
};


class Fairy2 : public Enemy, public BaseAttributes<Fairy2>
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

	inline SpaceFloat getMass() const { return 40.0; }

	inline string getSprite() const { return "redFairy2"; }

	virtual void initStateMachine();
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

class IceFairy :
	public Enemy,
	public BaseAttributes<IceFairy>
{
public:
	static const string baseAttributes;

	IceFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	inline SpaceFloat getMass() const { return 40.0; }

	inline string getSprite() const { return "blueFairy2"; }

	virtual void initStateMachine();
};

#endif /* Fairy_hpp */
