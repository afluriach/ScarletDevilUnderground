//
//  MiscMagicEffects.hpp
//  Koumachika
//
//  Created by Toni on 3/12/19.
//
//

#ifndef MiscMagicEffects_hpp
#define MiscMagicEffects_hpp

#include "MagicEffect.hpp"

class TeleportPad;

class Teleport : public MagicEffect {
public:
	static constexpr effect_flags flags = enum_bitwise_or(effect_flags, indefinite, active);

	Teleport(effect_params params, float magnitude, float length);
	inline virtual ~Teleport() {}

	virtual void init();
	virtual void update();
	virtual void end();
protected:
	vector<object_ref<TeleportPad>> targets;
	object_ref<TeleportPad> toUse;
};

#endif /* MiscMagicEffects_hpp */
