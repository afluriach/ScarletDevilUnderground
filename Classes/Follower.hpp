//
//  Follower.hpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#ifndef Follower_hpp
#define Follower_hpp

#include "AI.hpp"
#include "AIMixins.hpp"
#include "Agent.hpp"

class Follower : public Agent, public TouchDamageEnemy
{
public:
    inline Follower(const ValueMap& args) :
	GObject(args),
    Agent(args)
    {}
    
    virtual void onPlayerBulletHit(Bullet* bullet);

    virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 40.0f;}
    virtual inline GType getType() const {return GType::enemy;}

    inline string imageSpritePath() const {return "sprites/reisen.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
        
    virtual inline float getMaxSpeed() const {return 1.0f;}
    virtual inline float getMaxAcceleration() const {return 4.5f;}

	virtual inline shared_ptr<ai::State> getStartState() {
		return make_shared<ai::FollowerState>(); }

    GObject* target = nullptr;
    
    void init();
    void update();
};

#endif /* Follower_hpp */
