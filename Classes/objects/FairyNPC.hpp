//
//  FairyNPC.hpp
//  Koumachika
//
//  Created by Toni on 3/27/19.
//
//

#ifndef FairyNPC_hpp
#define FairyNPC_hpp

#include "NPC.hpp"

class FairyMaid : public NPC
{
public:
	static const string baseAttributes;

	FairyMaid(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 3.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline string getSprite() const { return "blueFairy"; }
};

class BlueFairyNPC : public NPC
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	BlueFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args);

	inline string getSprite() const { return "blueFairy"; }

	inline virtual bool isDialogAvailable() { return true; }
	virtual string getDialog();
	virtual void onDialogEnd();
protected:
	int level;
};

class GhostFairyNPC : public NPC
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	GhostFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 3.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	inline string getSprite() const { return "ghostFairy"; }

	virtual inline string initStateMachine() { return "ghost_fairy_npc"; }
};

#endif /* FairyNPC_hpp */
