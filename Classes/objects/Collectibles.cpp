//
//  Collectibles.cpp
//  Koumachika
//
//  Created by Toni on 4/14/18.
//
//

#include "Prefix.h"

#include "Collectibles.hpp"
#include "MagicEffect.hpp"
#include "Player.hpp"
#include "value_map.hpp"

collectible_properties::~collectible_properties()
{
}

ObjectGeneratorType Collectible::create(GSpace* space, string id, SpaceVect pos)
{
	vector<string> tokens;
	string actual;
	if (id.empty())
		return GObject::null_object_factory();

	tokens = splitString(id, ",");

	if (tokens.size() > 1) {
		int idx = space->getRandomInt(0, tokens.size() - 1);
		actual = tokens[idx];
	}
	else {
		actual = id;
	}

	return GObject::make_object_factory<Collectible>(pos, actual);
}

Collectible::Collectible(GSpace* space, ObjectIDType id, SpaceVect pos, string collectibleID) :
	GObject(
		space,
		id,
		object_params(pos, SpaceVect::zero, float_pi*0.5),
		physics_params(
			GType::item,
			PhysicsLayers::onGround,
			SpaceVect(0.5,0.5),
			-1.0,
			true
		)
	)
{
	collectible_properties props = app::getCollectible(collectibleID);

	if (!props.effect) {
		log("Invalid Collectible ID: %s", collectibleID);
	}

	effect = props.effect;
	sprite = props.sprite;
	attrs = props.attr;

	attrs.type = DamageType::item;
}

Collectible::~Collectible()
{
}

shared_ptr<sprite_properties> Collectible::getSprite() const
{
	return app::getSprite(sprite);
}

string Collectible::itemName() const {
	return "";
}

local_shared_ptr<MagicEffectDescriptor> Collectible::getEffect(GObject* target) const
{
	return effect;
}

void Collectible::onPlayerContact(Player* p)
{
	if (canAcquire(p))
		onAcquire(p);
}

bool Collectible::canAcquire(Player* player) {
	return effect && effect->canApply(player, attrs);
}

//If this is an invalid Collectible (null effect), canAcquire should return false
//and this code shouldn't run.
void Collectible::onAcquire(Player* player) {
	player->applyMagicEffect(effect, attrs);
	space->removeObject(this);
	playSoundSpatial("sfx/powerup.wav");
}
