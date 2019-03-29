//
//  RumiaSpells.hpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#ifndef RumiaSpells_hpp
#define RumiaSpells_hpp

#include "Spell.hpp"

class DarknessSignDemarcation : public Spell
{
public:
	static const string name;
	static const string description;
	static const float cost;

	static const SpaceFloat betweenBurstDelay;
	static const SpaceFloat burstInterval;
	static const SpaceFloat launchDist;
	static const SpaceFloat angleSkew;
	static const int burstCount;
	static const int bulletsPerBurst;

	DarknessSignDemarcation(GObject* caster);
	inline virtual ~DarknessSignDemarcation() {}

	GET_DESC(DarknessSignDemarcation)
	inline virtual void init() {}
	virtual void update();
	inline virtual void end() {}
protected:
	void generate();

	SpaceFloat timer = betweenBurstDelay;
	int crntBurst = 0;
};

class DarknessSignDemarcation2 : public Spell
{
public:
	static const string name;
	static const string description;
	static const float cost;

	static const SpaceFloat betweenBurstDelay;
	static const SpaceFloat burstInterval;
	static const SpaceFloat launchDist;
	static const SpaceFloat angleSkew;
	static const int burstCount;
	static const int bulletsPerBurst;

	DarknessSignDemarcation2(GObject* caster);
	inline virtual ~DarknessSignDemarcation2() {}

	GET_DESC(DarknessSignDemarcation2)
	inline virtual void init() {}
	virtual void update();
	inline virtual void end() {}
protected:
	void generate();

	SpaceFloat timer = 0.0;
	int crntBurst = 0;
};

class NightSignPinwheel : public Spell
{
public:
	static const string name;
	static const string description;
	static const float cost;

	static const int legCount;
	static const int bulletsPerLegCount;
	static const SpaceFloat launchDelay;
	static const SpaceFloat refreshRatio;
	static const SpaceFloat legLength;
	static const SpaceFloat legStartDist;
	static const SpaceFloat legAngleSkew;

	NightSignPinwheel(GObject* caster);
	inline virtual ~NightSignPinwheel() {}

	GET_DESC(NightSignPinwheel)
	inline virtual void init() {}
	virtual void update();
	virtual void end();
protected:
	SpaceFloat bulletsExistingRatio();
	void generate();
	void generateLeg(SpaceFloat angle);
	void launch();
	void removeBullets();

	unordered_set<gobject_ref> bullets;
	SpaceFloat timer = 0.0;
	bool waitingToLaunch = false;
};

#endif /* RumiaSpells_hpp */
