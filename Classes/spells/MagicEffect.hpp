//
//  MagicEffect.hpp
//  Koumachika
//
//  Created by Toni on 11/28/18.
//
//

#ifndef MagicEffect_hpp
#define MagicEffect_hpp

#include "object_ref.hpp"

class MagicEffect
{
public:
	enum class state
	{
		created = 1,
		active,
		ending,
		expired,
	};

	inline MagicEffect(gobject_ref target) : MagicEffect(target, 0.0f) {}
	MagicEffect(gobject_ref target, float magnitude);

	GSpace* getSpace() const;

	inline virtual ~MagicEffect() {}

	virtual void init() = 0;
	virtual void update() = 0;
	virtual void end() = 0;

	gobject_ref target;
	float magnitude;
	state crntState;
};

#endif /* MagicEffect_hpp */
