//
//  AttributeEffects.hpp
//  Koumachika
//
//  Created by Toni on 12/18/19.
//
//

#ifndef RestoreAttribute_hpp
#define RestoreAttribute_hpp

#include "MagicEffect.hpp"

class RestoreAttribute : public ImmediateMagicEffect
{
public:
	static bool canApply(GObject* target, float magnitude, Attribute attr);

	RestoreAttribute(GObject* target, float magnitude, Attribute attr);

	virtual void init();

	const Attribute attr;
};

#endif /* RestoreAttribute_hpp */
