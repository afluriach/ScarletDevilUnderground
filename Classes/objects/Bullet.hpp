//
//  Bullet.hpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Bullet_hpp
#define Bullet_hpp

#include "Attributes.hpp"
#include "GObject.hpp"
#include "types.h"

class Agent;
class Wall;

class Bullet : virtual public GObject
{
public:
	static const bool logRicochets;

	Bullet(Agent* agent);
	inline virtual ~Bullet() {}

	virtual inline bool getSensor() const { return true; }
	virtual inline SpaceFloat getMass() const { return 0.1; }
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::ground; }

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	//Interface
	virtual inline AttributeMap getAttributeEffect() const { return AttributeMap(); }
	virtual inline shared_ptr<MagicEffect> getMagicEffect(gobject_ref target) { return nullptr; }

	virtual void onWallCollide(Wall* wall);
	virtual void onEnvironmentCollide(GObject* obj);
	virtual void onAgentCollide(Agent* agent, SpaceVect n);
	virtual void onBulletCollide(Bullet* bullet);

	bool applyRicochet(SpaceVect n);

	void setBodyVisible(bool b);

	SpaceFloat knockback = 0.0;
	float agentAttackMultiplier = 1.0f;
	int ricochetCount = 0;
	int hitCount = 1;
};

#endif /* Bullet_hpp */
