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

	MagicEffect(gobject_ref target, float magnitude);

	inline virtual ~MagicEffect() {}

	virtual void init() = 0;
	virtual void update() = 0;
	virtual void end() = 0;

	bool combine(shared_ptr<MagicEffect> rhs);

	gobject_ref target;
	float magnitude;
	state crntState;
};

template<typename Derived>

class MagicEffectImpl : public MagicEffect
{
public:
	
	inline MagicEffectImpl(gobject_ref target, float magnitude) :
		MagicEffect(target, magnitude)
	{}
};

class FrostStatusEffect : public MagicEffectImpl<FrostStatusEffect>
{
public:
	FrostStatusEffect(gobject_ref target, float magnitude);

	virtual inline void init();
	virtual void update();
	virtual void end();
};

class FreezeStatusEffect : public MagicEffectImpl<FreezeStatusEffect>
{
public:
	FreezeStatusEffect(gobject_ref target, float magnitude);

	virtual inline void init();
	virtual void update();
	virtual void end();

	Action* spriteEffectAction = nullptr;
};

#endif /* MagicEffect_hpp */
