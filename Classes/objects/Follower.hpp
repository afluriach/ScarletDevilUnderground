//
//  Follower.hpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#ifndef Follower_hpp
#define Follower_hpp

#include "Agent.hpp"

class Follower : virtual public Agent, public TouchDamageEnemy, public BaseAttributes<Follower>
{
public:
	static const AttributeMap baseAttributes;

    inline Follower(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space,id,args),
    Agent(space,id,args)
    {}
    
    virtual void hit(int damage, shared_ptr<MagicEffect> effect);

    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 40.0f;}
    virtual inline GType getType() const {return GType::enemy;}

    inline string imageSpritePath() const {return "sprites/reisen.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
        
	virtual inline void initStateMachine(ai::StateMachine& sm) {
		sm.addThread(make_shared<ai::FollowerMain>());
	}

    GObject* target = nullptr;
    
    void init();
    void update();
};

#endif /* Follower_hpp */
