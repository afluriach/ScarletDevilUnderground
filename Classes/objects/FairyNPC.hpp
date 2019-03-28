//
//  FairyNPC.hpp
//  Koumachika
//
//  Created by Toni on 3/27/19.
//
//

#ifndef FairyNPC_hpp
#define FairyNPC_hpp

#include "Agent.hpp"
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
	virtual inline int pixelWidth() const { return 128; }

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
	virtual inline int pixelWidth() const { return 128; }

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
	virtual inline GType getRadarType() const { return GType::enemySensor; }
	virtual inline SpaceFloat getRadarRadius() const { return 3.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline string imageSpritePath() const { return "sprites/ghost-fairy.png"; }
	virtual bool isAgentAnimation() const { return true; }
	virtual inline int pixelWidth() const { return 128; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

#endif /* FairyNPC_hpp */
