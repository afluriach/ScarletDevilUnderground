//
//  Follower.hpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#ifndef Follower_hpp
#define Follower_hpp

#include "Enemy.hpp"

class Follower : public Enemy, public BaseAttributes<Follower>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(Follower);

	virtual void hit(AttributeMap attributeEffects, shared_ptr<MagicEffect> effect);

    inline SpaceFloat getMass() const {return 40.0;}

    inline string imageSpritePath() const {return "sprites/reisen.png";}
        
	virtual void initStateMachine(ai::StateMachine& sm);    
};

class FollowerMain : public ai::Function {
public:
	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	FuncGetName(FollowerMain)
protected:
	gobject_ref target = nullptr;
};

#endif /* Follower_hpp */
