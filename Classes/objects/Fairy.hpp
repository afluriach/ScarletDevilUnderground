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

class FairyMaid :
	virtual public Agent,
	public DialogImpl,
	public AIPackage<FairyMaid>,
	public BaseAttributes<FairyMaid>
{
public:
	static const AttributeMap baseAttributes;
	static const AIPackage<FairyMaid>::AIPackageMap aiPackages;

	FairyMaid(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 3.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
	virtual inline SpaceFloat getMass() const { return 10.0; }
	virtual inline GType getType() const { return GType::npc; }
	virtual inline GType getRadarType() const { return GType::enemySensor; }

	inline string imageSpritePath() const { return "sprites/fairy-blue.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	void flee_player(ai::StateMachine& sm, const ValueMap& args);
	void idle(ai::StateMachine& sm, const ValueMap& args);
	void wander(ai::StateMachine& sm, const ValueMap& args);
};

class BlueFairyNPC :
	virtual public Agent,
	public NoAttributes,
	public DialogEntity
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	BlueFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getMass() const { return -1.0; }
	virtual inline GType getType() const { return GType::npc; }

	inline string imageSpritePath() const { return "sprites/fairy-blue.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	inline virtual bool isDialogAvailable() { return true; }
	virtual string getDialog();
	virtual void onDialogEnd();
protected:
	int level;
};

class GhostFairyNPC :
	virtual public Agent,
	public NoAttributes
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	GhostFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getMass() const { return 10.0; }
	virtual inline GType getType() const { return GType::npc; }
	virtual inline bool getSensor() const { return true; }

	inline string imageSpritePath() const { return "sprites/ghost-fairy.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

class Fairy1 :
public Enemy,
public AIPackage<Fairy1>,
public AttributesPackage<Fairy1>
{
public:
	static const AttributeMap baseAttributes;
	static const AttributePackageMap attributePackages;
	static const AIPackage<Fairy1>::AIPackageMap aiPackages;

	Fairy1(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
	virtual inline SpaceFloat getMass() const { return 40.0; }

	inline string imageSpritePath() const { return "sprites/fairy-green.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	void maintain_distance(ai::StateMachine& sm, const ValueMap& args);
	void circle_and_fire(ai::StateMachine& sm, const ValueMap& args);
	void circle_around_point(ai::StateMachine& sm, const ValueMap& args);
	void flock(ai::StateMachine& sm, const ValueMap& args);
};

class BlueFairy :
	public Enemy,
	public AIPackage<BlueFairy>,
	public BaseAttributes<BlueFairy>
{
public:
	static const AIPackage<BlueFairy>::AIPackageMap aiPackages;
	static const AttributeMap baseAttributes;

	BlueFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 5.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
	virtual inline SpaceFloat getMass() const { return 25.0; }

	inline string imageSpritePath() const { return "sprites/fairy-blue.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	void follow_path(ai::StateMachine& sm, const ValueMap& args);
};

class RedFairy :
	public Enemy,
	public BaseAttributes<RedFairy>,
	public RegisterUpdate<RedFairy>
{
public:
	static const AttributeMap baseAttributes;

	RedFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	void update();

	virtual inline SpaceFloat getRadarRadius() const { return 3.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
	inline SpaceFloat getMass() const { return 25.0; }

	inline string imageSpritePath() const { return "sprites/fairy-red.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

class GreenFairy :
	public Enemy,
	public BaseAttributes<GreenFairy>
{
public:
	static const AttributeMap baseAttributes;

	GreenFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline SpaceFloat getMass() const { return 25.0; }

	inline string imageSpritePath() const { return "sprites/fairy-green.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

class ZombieFairy :
	public Enemy,
	public BaseAttributes<ZombieFairy>,
	public RegisterInit<ZombieFairy>
{
public:
	static const AttributeMap baseAttributes;

	ZombieFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	void init();

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline SpaceFloat getMass() const { return 25.0; }

	inline string imageSpritePath() const { return "sprites/zombie-fairy.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);
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
		supportOffered,
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
	void addFleeThread();
	void addSupportThread(object_ref<Fairy2> other);
	void removeSupportThread();

	void update();

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
	static const AttributeMap baseAttributes;

	IceFairy(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	inline SpaceFloat getMass() const { return 40.0; }

	inline string imageSpritePath() const { return "sprites/fairy2-blue.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 512; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

#endif /* Fairy_hpp */
