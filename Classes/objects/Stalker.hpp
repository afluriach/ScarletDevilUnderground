//
//  Stalker.hpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#ifndef Stalker_hpp
#define Stalker_hpp

#include "AI.hpp"
#include "Collectibles.hpp"
#include "Enemy.hpp"
#include "macros.h"

class Stalker : public Enemy, public BaseAttributes<Stalker>, public RegisterUpdate<Stalker>
{
public:
	static const AttributeMap baseAttributes;

	Stalker(GSpace* space, ObjectIDType id, const ValueMap& args);

    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    virtual inline SpaceFloat getMass() const {return 40.0;}

	virtual inline string imageSpritePath() const {return "sprites/dark_cirno.png";}

	virtual void initStateMachine(ai::StateMachine& sm);
	virtual void teleport(SpaceVect pos);

	void update();
protected:
	SpaceVect prev_pos;
};

class StalkerMain : public ai::Function {
public:
	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	FuncGetName(StalkerMain)
};


class StalkerTeleport : public ai::Cast1 {
public:
	StalkerTeleport(const vector<object_ref<TeleportPad>>& targets);
	FuncGetName(StalkerMain)
};


#endif /* Stalker_hpp */
