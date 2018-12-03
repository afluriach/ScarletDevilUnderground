//
//  Agent.cpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "GSpace.hpp"

 Agent::Agent(const ValueMap& args) :
	GObject(args),
	PatchConSprite(args),
	StateMachineObject(args),
	RegisterUpdate<Agent>(this),
	RegisterInit<Agent>(this)
{}

 void Agent::init() {
	 initStateMachine(fsm);

	 attributeSystem = getBaseAttributes();

	 power = attributeSystem.getAdjustedValue(Attribute::power);
	 health = attributeSystem.getAdjustedValue(Attribute::health);
 }

 float Agent::getMaxSpeed() const
 {
	 return attributeSystem.getAdjustedValue(Attribute::speed);
 }

 float Agent::getMaxAcceleration() const
 {
	 return attributeSystem.getAdjustedValue(Attribute::acceleration);
 }

 float Agent::getMaxPower() const
 {
	 return attributeSystem.getAdjustedValue(Attribute::power);
 }

 float Agent::getMaxHealth() const
 {
	 return attributeSystem.getAdjustedValue(Attribute::health);
 }

 void Agent::hit(int damage, shared_ptr<MagicEffect> effect)
 {
	 health -= damage;
	 if (health < 0) health = 0;

	 if (effect)
		 addMagicEffect(effect);

	 Enemy* _enemy = dynamic_cast<Enemy*>(this);

	 if(_enemy)
		 _enemy->runDamageFlicker();
 }

 void Agent::update()
 {
	 if (health <= 0 && getMaxHealth() != 0) {
		 app->space->removeObject(this);
	 }
 }

 const AttributeMap GenericAgent::baseAttributes = boost::assign::map_list_of
	(Attribute::speed, 1.5f)
	(Attribute::acceleration, 6.0f)
	 ;

 GenericAgent::GenericAgent(const ValueMap& args) :
	 GObject(args),
	 Agent(args)
 {
	 spriteName = args.at("sprite").asString();
 }

 void GenericAgent::initStateMachine(ai::StateMachine& sm)
 {
    auto wanderThread = make_shared<ai::Thread>(
        make_shared<ai::Wander>(),
        &fsm,
        0,
        make_enum_bitfield(ai::ResourceLock::movement)
    );
    
    auto detectThread = make_shared<ai::Thread>(
        make_shared<ai::Detect>(
            "player",
            [=](GObject* target) -> shared_ptr<ai::Function> {
                return make_shared<ai::Flee>(target, 3.0f);
            }
        ),
        &fsm,
        1,
        bitset<ai::lockCount>()
    );
    
    wanderThread->setResetOnBlock(true);
    
    fsm.addThread(wanderThread);
    fsm.addThread(detectThread);
 }
