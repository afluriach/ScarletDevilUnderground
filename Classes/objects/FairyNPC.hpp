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

class FairyMaid :
	virtual public Agent,
	public DialogImpl,
	public BaseAttributes<FairyMaid>
{
public:
	static const string baseAttributes;

	FairyMaid(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 3.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
	virtual inline SpaceFloat getMass() const { return 10.0; }
	virtual inline GType getType() const { return GType::npc; }
	virtual inline GType getRadarType() const { return GType::enemySensor; }

	inline string getSprite() const { return "blueFairy"; }
};

class BlueFairyNPC :
	virtual public Agent,
	public DialogEntity
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	BlueFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getMass() const { return -1.0; }
	virtual inline GType getType() const { return GType::npc; }

	inline string getSprite() const { return "blueFairy"; }

	inline virtual bool isDialogAvailable() { return true; }
	virtual string getDialog();
	virtual void onDialogEnd();
protected:
	int level;
};

class GhostFairyNPC :
	virtual public Agent
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	GhostFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getMass() const { return 10.0; }
	virtual inline GType getType() const { return GType::npc; }
	virtual inline GType getRadarType() const { return GType::enemySensor; }
	virtual inline SpaceFloat getRadarRadius() const { return 3.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline string getSprite() const { return "ghostFairy"; }

	virtual inline string initStateMachine() { return "ghost_fairy_npc"; }
};

#endif /* FairyNPC_hpp */
