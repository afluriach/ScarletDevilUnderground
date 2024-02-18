//
//  MagicEffectImpl.hpp
//  Koumachika
//
//  Created by Toni on 2/17/24.
//

#ifndef MagicEffectImpl_hpp
#define MagicEffectImpl_hpp

#include "MagicEffect.hpp"

struct transformation_properties
{
	AttributeMap attribute_modifers;
	shared_ptr<sprite_properties> sprite;
	float sprite_scale = 1.0f;
	
	bool flying = false;
};

class Transformation : public AgentEffect
{
public:
	static constexpr effect_flags flags = enum_bitwise_or(effect_flags, durable, agent);

	Transformation(effect_params params, transformation_properties props);

	virtual void init();
	virtual void end();
protected:
	transformation_properties props;
};

#endif /* MagicEffectImpl_hpp */
