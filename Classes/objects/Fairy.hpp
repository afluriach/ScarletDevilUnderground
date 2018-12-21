//
//  Fairy.hpp
//  Koumachika
//
//  Created by Toni on 11/30/18.
//
//

#ifndef Fairy_hpp
#define Fairy_hpp

#include "Agent.hpp"
#include "macros.h"

class Fairy1 :
virtual public Agent,
public BaseAttributes<Fairy1>
{
public:
	static const AttributeMap baseAttributes;

	inline Fairy1(GSpace* space, ObjectIDType id, const ValueMap& args) :
		MapObjForwarding(GObject),
		MapObjForwarding(Agent)
	{}

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline GType getRadarType() const { return GType::playerSensor; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	virtual inline SpaceFloat getRadius() const { return 0.35; }
	inline SpaceFloat getMass() const { return 40.0; }
	virtual inline GType getType() const { return GType::enemy; }

	inline string imageSpritePath() const { return "sprites/melancholy.png"; }
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

class Fairy2 :
	virtual public Agent,
	public RegisterUpdate<Fairy2>,
	public BaseAttributes<Fairy2>
{
public:
	static const AttributeMap baseAttributes;
	static const boost::rational<int> lowHealthRatio;

	inline Fairy2(GSpace* space, ObjectIDType id, const ValueMap& args) :
		MapObjForwarding(GObject),
		MapObjForwarding(Agent),
		RegisterUpdate<Fairy2>(this)
	{}

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline GType getRadarType() const { return GType::playerSensor; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	virtual inline SpaceFloat getRadius() const { return 0.35; }
	inline SpaceFloat getMass() const { return 40.0; }
	virtual inline GType getType() const { return GType::enemy; }

	inline string imageSpritePath() const { return "sprites/melancholy.png"; }
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void initStateMachine(ai::StateMachine& sm);

	void update();

	void requestHandler(object_ref<Fairy2> other);
protected:
	shared_ptr<ai::TrackByType<Fairy2>> trackFunction;
	bool requestSent = false;
};

class IceFairy :
	virtual public Agent,
	public BaseAttributes<IceFairy>
{
public:
	static const AttributeMap baseAttributes;

	inline IceFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
		MapObjForwarding(GObject),
		MapObjForwarding(Agent)
	{}

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline GType getRadarType() const { return GType::playerSensor; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	virtual inline SpaceFloat getRadius() const { return 0.35; }
	inline SpaceFloat getMass() const { return 40.0; }
	virtual inline GType getType() const { return GType::enemy; }

	inline string imageSpritePath() const { return "sprites/dark_cirno.png"; }
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

#endif /* Fairy_hpp */
