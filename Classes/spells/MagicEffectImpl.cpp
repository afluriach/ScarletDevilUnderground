//
//  MagicEffectImpl.cpp
//  Koumachika
//
//  Created by Toni on 2/17/24.
//

#include "Prefix.h"

#include "MagicEffectImpl.hpp"

Transformation::Transformation(effect_params params, transformation_properties props) :
	AgentEffect(params),
	props(props)
{}

void Transformation::init()
{
	agent->setSprite(props.sprite);
	agent->sprite.setScale(props.sprite_scale);
	
	for(auto entry : props.attribute_modifers){
		agent->modifyAttribute(entry.first, entry.second);
	}
	
	agent->setIsOnFloor(!props.flying);
}

void Transformation::end()
{
	agent->setSprite(agent->getOriginalSprite());
	agent->sprite.setScale(1.0f);
	
	for(auto entry : props.attribute_modifers){
		agent->modifyAttribute(entry.first, -entry.second);
	}
	
	agent->setIsOnFloor(!agent->isFlyingProperty());
}
