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

	inline string getSprite() const { return "blueFairy"; }

	void flee_player(const ValueMap& args);
	void idle(const ValueMap& args);
	void wander(const ValueMap& args);
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

	inline string getSprite() const { return "blueFairy"; }

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

	inline string getSprite() const { return "ghostFairy"; }

	virtual void initStateMachine();
};

#endif /* FairyNPC_hpp */
