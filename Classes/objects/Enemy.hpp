//
//  Enemy.hpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#ifndef Enemy_hpp
#define Enemy_hpp

#include "Agent.hpp"
#include "Collectibles.hpp"

struct enemy_properties
{
	string name;
	string typeName;
	string sprite;
	string attributes;
	string ai_package;
	string firepattern;
	string effects;

	//Strictly radius for now, as Enemy->Agent still is a CircleBody
	SpaceFloat radius = 0.0;
	SpaceFloat mass = 0.0;
	SpaceFloat viewRange = 0.0;
	SpaceFloat viewAngle = 0.0;

	DamageInfo touchEffect;
	string collectible;

	shared_ptr<LightArea> lightSource;

	bool detectEssence = false;
	bool isFlying = false;
};

class Enemy : public Agent
{
public:
	Enemy(
		GSpace* space,
		ObjectIDType id,
		const ValueMap& args,
		const string& baseAttributes,
		SpaceFloat radius,
		SpaceFloat mass,
		string drop_id,
		bool isFlying
	);

	inline virtual ~Enemy() {}

	void runDamageFlicker();

	virtual bool hit(DamageInfo damage);
	virtual void onRemove();
protected:
	string drop_id;
};

class EnemyImpl : public Enemy
{
public:
	EnemyImpl(
		GSpace* space, ObjectIDType id, const ValueMap& args,
		shared_ptr<enemy_properties> props
	);

	virtual void init();

	virtual DamageInfo touchEffect() const;
	virtual AttributeMap getBaseAttributes() const;

	virtual bool hasEssenceRadar() const;
	virtual SpaceFloat getRadarRadius() const;
	virtual SpaceFloat getDefaultFovAngle() const;

	virtual string getSprite() const;
	virtual shared_ptr<LightArea> getLightSource() const;

	virtual string initStateMachine();

	virtual string getTypeName() const;
	virtual string getProperName() const;
protected:
	void loadEffects();

	shared_ptr<enemy_properties> props;
};

#endif /* Enemy_hpp */
