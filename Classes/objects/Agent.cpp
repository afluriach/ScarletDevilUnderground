//
//  Agent.cpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#include "Prefix.h"

#include "Agent.hpp"

 Agent::Agent(const ValueMap& args) :
	GObject(args),
	PatchConSprite(args),
	StateMachineObject(args),
	RegisterInit<Agent>(this)
{}

 void Agent::init() {
	 initStateMachine(fsm);
 }

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
