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

	virtual bool hit(DamageInfo damage);

    inline SpaceFloat getMass() const {return 40.0;}

    inline string imageSpritePath() const {return "sprites/reisen.png";}
        
	virtual void initStateMachine();    
};

class FollowerMain : public ai::Function {
public:
	inline FollowerMain(ai::StateMachine* fsm) : ai::Function(fsm) {}

	virtual void onEnter();
	virtual ai::update_return update();
	FuncGetName(FollowerMain)
protected:
	gobject_ref target = nullptr;
};

#endif /* Follower_hpp */
