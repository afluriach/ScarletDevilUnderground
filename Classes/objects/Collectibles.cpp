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
	GObject(PosAngleParams(pos, float_pi * 0.5), physics_params(GType::playerPickup, PhysicsLayers::onGround, SpaceVect(0.5,0.5), -1.0, true))
{
	collectible_properties props = app::getCollectible(collectibleID);

	if (!props.effect) {
		log("Invalid Collectible ID: %s", collectibleID);
	}

	effect = props.effect;
	sprite = props.sprite;

	effectLength = props.length;
	effectMagnitude = props.magnitude;
}

string Collectible::getSprite() const
{
	return sprite;
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
	return effect && effect->canApply(player, effectMagnitude, effectLength);
}

//If this is an invalid Collectible (null effect), canAcquire should return false
//and this code shouldn't run.
void Collectible::onAcquire(Player* player) {
	player->applyMagicEffect(effect, effectMagnitude, effectLength);
	space->removeObject(this);
	playSoundSpatial("sfx/powerup.wav");
}
