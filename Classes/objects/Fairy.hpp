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
#include "macros.h"

class Fairy1 :
public Enemy,
public BaseAttributes<Fairy1>
{
public:
	static const AttributeMap baseAttributes;

	Fairy1(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	inline SpaceFloat getMass() const { return 40.0; }

	inline string imageSpritePath() const { return "sprites/fairy-blue.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

class Fairy1A :
	public Enemy,
	public BaseAttributes<Fairy1A>
{
public:
	static const AttributeMap baseAttributes;

	Fairy1A(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	inline SpaceFloat getMass() const { return 40.0; }

	inline string imageSpritePath() const { return "sprites/fairy-gold.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

class Fairy1B :
	public Enemy,
	public BaseAttributes<Fairy1B>
{
public:
	static const AttributeMap baseAttributes;

	Fairy1B(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	inline SpaceFloat getMass() const { return 40.0; }

	inline string imageSpritePath() const { return "sprites/fairy-green.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);
protected:
	string waypointName;
};

class Fairy2 :
	public Enemy,
	public RegisterUpdate<Fairy2>,
	public BaseAttributes<Fairy2>
{
public:

	enum class ai_state {
		normal,
		flee,
		fleeWithSupport,
		supporting
	};

	enum class ai_priority {
		engage,
		support,
		flee,
	};

	static const AttributeMap baseAttributes;
	static const boost::rational<int> lowHealthRatio;

	Fairy2(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	inline SpaceFloat getMass() const { return 40.0; }

	inline string imageSpritePath() const { return "sprites/fairy2-red.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);

	void update();

	object_ref<Fairy2> requestHandler(object_ref<Fairy2> other);
	void responseHandler(object_ref<Fairy2> supporting);
	void cancelRequest();
protected:
	shared_ptr<ai::TrackByType<Fairy2>> trackFunction;
	ai_state crntState = ai_state::normal;
	unsigned int supportThread = 0;
};

class IceFairy :
	public Enemy,
	public BaseAttributes<IceFairy>
{
public:
	static const AttributeMap baseAttributes;

	inline IceFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
		MapObjForwarding(GObject),
		MapObjForwarding(Agent),
		Enemy(collectible_id::power1)
	{}

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	inline SpaceFloat getMass() const { return 40.0; }

	inline string imageSpritePath() const { return "sprites/fairy2-blue.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

#endif /* Fairy_hpp */
