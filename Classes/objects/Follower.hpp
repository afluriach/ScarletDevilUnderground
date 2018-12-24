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

    inline Follower(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space,id,args),
    Agent(space,id,args)
    {}
    
    virtual void hit(int damage, shared_ptr<MagicEffect> effect);

    inline SpaceFloat getMass() const {return 40.0;}

    inline string imageSpritePath() const {return "sprites/reisen.png";}
        
	virtual inline void initStateMachine(ai::StateMachine& sm) {
		sm.addThread(make_shared<ai::FollowerMain>());
	}

    GObject* target = nullptr;
    
    void init();
    void update();
};

#endif /* Follower_hpp */
