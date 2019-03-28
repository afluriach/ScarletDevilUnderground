//
//  SpellUtil.hpp
//  Koumachika
//
//  Created by Toni on 3/27/19.
//
//

#ifndef SpellUtil_hpp
#define SpellUtil_hpp

#include "Attributes.hpp"
#include "types.h"

class GObject;

void explosion(const GObject* source, SpaceFloat radius, AttributeMap baseEffect);
float getExplosionScale(const GObject* source, const GObject* target, SpaceFloat radius);
void applyKnockback(const GObject* source, GObject* target, SpaceFloat mag);

#endif /* SpellUtil_hpp */
