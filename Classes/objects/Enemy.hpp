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

	//Strictly radius for now, as Enemy->Agent still is a CircleBody
	SpaceFloat radius = 0.0;
	SpaceFloat mass = 0.0;
	SpaceFloat viewRange = 0.0;
	SpaceFloat viewAngle = 0.0;

	DamageInfo touchEffect;
	collectible_id collectible = collectible_id::nil;

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
		collectible_id drop_id
	);

	inline virtual ~Enemy() {}

	void runDamageFlicker();

	virtual bool hit(DamageInfo damage);
	virtual void onRemove();

	virtual inline GType getType() const { return GType::enemy; }
protected:
	collectible_id drop_id = collectible_id::nil;
};

class EnemyImpl : public Enemy
{
public:
	EnemyImpl(
		GSpace* space, ObjectIDType id, const ValueMap& args,
		shared_ptr<enemy_properties> props
	);

	virtual DamageInfo touchEffect() const;
	virtual AttributeMap getBaseAttributes() const;

	virtual bool hasEssenceRadar() const;
	virtual SpaceFloat getRadarRadius() const;
	virtual SpaceFloat getDefaultFovAngle() const;

	virtual PhysicsLayers getLayers() const;

	virtual string getSprite() const;
	virtual shared_ptr<LightArea> getLightSource() const;

	virtual string initStateMachine();

	virtual string getTypeName() const;
	virtual string getProperName() const;
protected:
	shared_ptr<enemy_properties> props;
};

#endif /* Enemy_hpp */
