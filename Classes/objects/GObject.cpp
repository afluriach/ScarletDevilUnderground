//
//  GObject.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "app_constants.hpp"
#include "audio_context.hpp"
#include "enum.h"
#include "FloorSegment.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "LuaAPI.hpp"
#include "MagicEffect.hpp"
#include "physics_context.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "value_map.hpp"

unordered_map<type_index, string> GObject::typeNameMap;

GObject::GObject(shared_ptr<object_params> params) :
	space(params->space),
	name(params->name),
	uuid(params->id),
	prevPos(params->pos),
	prevAngle(params->angle),
	hidden(params->hidden)
{
	setInitialAngle(params->angle);
	if (params->vel != SpaceVect::zero) setInitialVelocity(params->vel);
	if (params->angularVel != 0.0) setInitialAngularVelocity(params->angularVel);
}

GObject::~GObject()
{
}

void GObject::removePhysicsObjects()
{
	space->physicsContext->removeObject(bodyShape, body, getMass() <= 0.0);
}

void GObject::removeGraphics(bool removeSprite)
{
	if (removeSprite && spriteID != 0) {
		space->addGraphicsAction(&graphics_context::removeSprite, spriteID);
	}
	if (drawNodeID != 0) {
		space->addGraphicsAction(&graphics_context::removeSprite, drawNodeID);
	}
	if (lightID != 0) {
		space->addGraphicsAction(&graphics_context::removeLightSource, lightID);
	}
}

GObject* GObject::constructByType(GSpace* space, ObjectIDType id, const string& type, const ValueMap& args )
{
    auto it = objectInfo.find(type);
    
    if(it != objectInfo.end()){
        AdapterType adapter =  it->second.consAdapter;
        return adapter(space, id, args);
    }
    else{
        log("Unknown object type %s!", type.c_str());
        return nullptr;
    }
}

ObjectGeneratorType GObject::factoryMethodByType(const string& type, const ValueMap& args)
{
	return [type,args](GSpace* space, ObjectIDType id) -> GObject* {
		return constructByType(space, id, type, args);
	};
}

string GObject::properNameByType(type_index t)
{
	const object_info* info = getObjectInfo(t);
	if (info)
		return info->properName;
	else
		return "";
}

const GObject::object_info* GObject::getObjectInfo(string name)
{
	auto it = objectInfo.find(name);
	if (it != objectInfo.end())
		return &it->second;
	else
		return nullptr;
}

const GObject::object_info* GObject::getObjectInfo(type_index t)
{
	auto it = typeNameMap.find(t);
	if (it != typeNameMap.end())
		return getObjectInfo(it->second);
	else
		return nullptr;
}

type_index GObject::getTypeIndex(string name)
{
	const object_info* info = getObjectInfo(name);

	if (info)
		return info->type;
	else
		return typeid(GObject);
}

void GObject::initNameMap()
{
	for (auto entry : objectInfo)
	{
		typeNameMap.insert_or_assign(entry.second.type, entry.first);
	}
}

string GObject::getTypeName() const
{
	return getOrDefault<type_index, string>(typeNameMap, typeid(*this), "GObject");
}

string GObject::getProperName() const
{
	return properNameByType(typeid(*this));
}

//BEGIN LOGIC

void GObject::init()
{
	initLightSource();
}

void GObject::update()
{
	updateSpells();
	updateMagicEffects();
	updateFloorSegment();
	
	updateParametricMove();

	prevPos = getPos();
	prevAngle = getAngle();

	updateFSM();
}

void GObject::onPitfall()
{
	space->removeObjectWithAnimation(this, pitfallShrinkAction());
}

void GObject::updateFSM() {
	if (fsm && !isFrozen)
		fsm->update();
}

unsigned int GObject::addThread(shared_ptr<ai::Function> threadMain) {
	if (!fsm) {
		fsm = make_unique<ai::StateMachine>(this);
	}

	return fsm->addThread(threadMain);
}

void GObject::removeThread(unsigned int uuid) {
	if (fsm) {
		fsm->removeThread(uuid);
	}
}

void GObject::removeThread(const string& name) {
	if (fsm) {
		fsm->removeThread(name);
	}
}

void GObject::printFSM() {
	if (fsm) {
		log("%s", fsm->toString());
	}
}

void GObject::setFrozen(bool val) {
	isFrozen = val;
}

//END LOGIC

//BEGIN PHYSICS

void GObject::launch() {
	setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));
}

void GObject::setInitialVelocity(const SpaceVect& v){
	space->addInitAction( [v, this]() -> void {
		setVel(v);
	} );
}

void GObject::setInitialAngle(SpaceFloat a){
	space->addInitAction( [a, this]() -> void {
		setAngle(a);
	} );
}

void GObject::setInitialAngularVelocity(SpaceFloat w){
	space->addInitAction( [w, this]() -> void{
		setAngularVel(w);
	} );
}

void GObject::setInitialDirectionOrDefault(const ValueMap& args, Direction d)
{
	Direction result = d;

	auto it = args.find("direction");
	if (it != args.end()) {
		Direction dir = stringToDirection(it->second.asString());
		if (dir != Direction::none)
			result = dir;
	}

	setInitialAngle(dirToPhysicsAngle(result));
}

Vec2 GObject::getInitialCenterPix()
{
    SpaceVect centerPix(prevPos);
    centerPix *= app::pixelsPerTile;
        
    return toCocos(centerPix);
}

SpaceVect GObject::getPos() const {
    return cpBodyGetPos(body);
}

SpaceVect GObject::getDeltaPos() const {
	return getPos() - prevPos;
}

void GObject::setPos(SpaceVect p){
	cpBodySetPos(body, p);
}
    
void GObject::setAngle(SpaceFloat a){
	cpBodySetAngle(body, canonicalAngle(a));
}
    
SpaceFloat GObject::getAngle() const {
    return canonicalAngle(cpBodyGetAngle(body));
}

void GObject::rotate(SpaceFloat a){
    setAngle(getAngle() + a );
}
    
SpaceVect GObject::getFacingVector() const{
    return SpaceVect::ray(1.0, getAngle());
}
    
void GObject::setDirection(Direction d) {
    if(body && d != Direction::none)
        setAngle(dirToPhysicsAngle(d));
}
    
SpaceVect GObject::getVel() const {
    return cpBodyGetVel(body);
}
    
void GObject::setVel(SpaceVect v) {
	cpBodySetVel(body, v);
}

SpaceFloat GObject::getAngularVel() const{
    return cpBodyGetAngVel(body);
}
    
void GObject::setAngularVel(SpaceFloat w){
	cpBodySetAngVel(body, w);
}

void GObject::applyForceForSingleFrame(SpaceVect f){
	cpBodyApplyImpulse(body, f * app::params.secondsPerFrame, SpaceVect::zero);
}

void GObject::applyImpulse(SpaceVect i) {
	cpBodyApplyImpulse(body, i, SpaceVect::zero);
}

void GObject::applyImpulse(SpaceFloat mag, SpaceFloat angle){
   applyImpulse(SpaceVect::ray(mag,angle));
}

void GObject::setParametricMove(parametric_space_function f)
{
	parametric_t = 0.0;
	parametric_f = f;
}

void GObject::removeParametricMove()
{
	parametric_t = -1.0;
	parametric_f = nullptr;
}

PhysicsLayers GObject::getCrntLayers() const
{
	return static_cast<PhysicsLayers>(bodyShape->layers);
}

void GObject::setLayers(PhysicsLayers layers)
{
	bodyShape->layers = to_uint(layers);
}

bool GObject::isOnFloor() const
{
	return bitwise_and(PhysicsLayers, getCrntLayers(), PhysicsLayers::floor) != PhysicsLayers::none;
}

SpaceVect GObject::getFloorVelocity() const
{
	return crntFloorCenterContact.isValid() ? crntFloorCenterContact.get()->getVel() : SpaceVect::zero;
}

void GObject::updateFloorSegment()
{
	if (getMass() <= 0.0 || !isOnFloor() || dynamic_cast<FloorSegment*>(this)) {
		return;
	}

	crntFloorCenterContact = nullptr;
	SpaceVect p = getPos();

	for (auto ref : crntFloorContacts)
	{
		//SpaceVect local = cpBodyWorld2Local(ref.get()->body, getPos());
		SpaceVect local = p - ref.get()->getPos();
		SpaceVect dim = ref.get()->getDimensions();
		if (abs(local.x) <= dim.x / 2.0 && abs(local.y) <= dim.y / 2.0) {
			crntFloorCenterContact = ref;
			break;
		}
	}

	if (crntFloorCenterContact.isValid()) {
		SpaceFloat _uk = uk();
		if (_uk > 0.0) {
			updateFriction(_uk * crntFloorCenterContact.get()->getFrictionCoeff());
		}
	}
	//If not touching any floor, check for pitfall.
	else if(crntFloorContacts.empty()){
		FloorSegment* pitfall = space->floorSegmentPointQuery(p);
		if (pitfall) pitfall->exclusiveFloorEffect(this);
	}
}

void GObject::updateFriction(float frictionCoeff)
{
	//linear
	SpaceVect vel = getVel();
	SpaceFloat force = getMass() * app::Gaccel * frictionCoeff;

	//if acceleraion, dv/dt, or change in velocity over one frame is greater
	//than current velocity, apply stop instead
	if (app::Gaccel * frictionCoeff * app::params.secondsPerFrame < vel.length())
		applyForceForSingleFrame(vel * -force);
	else
		setVel(SpaceVect::zero);

	//rotational
	SpaceFloat angularVel = getAngularVel();
	SpaceFloat angularImpulse = getMomentOfInertia() * app::Gaccel *  frictionCoeff * app::params.secondsPerFrame;

	if (angularImpulse < angularVel)
		setAngularVel(angularVel - angularImpulse);
	else
		setAngularVel(0);
}

void GObject::onContactFloorSegment(object_ref<FloorSegment> fs)
{
	if (fs.isValid() && isOnFloor())
	{
		if (crntFloorContacts.find(fs) != crntFloorContacts.end()) {
			log("onContactFloorSegment duplicate add attempted for floor ID %d", fs.getID());
			return;
		}

		fs.get()->onContact(this);
		crntFloorContacts.insert(fs);
	}
}

void GObject::onEndContactFloorSegment(object_ref<FloorSegment> fs)
{
	if (fs.isValid() && isOnFloor())
	{
		if (crntFloorContacts.find(fs) == crntFloorContacts.end()) {
			log("onEndContactFloorSegment floor ID %d not found", fs.getID());
			return;
		}

		fs.get()->onEndContact(this);
		crntFloorContacts.erase(fs);
	}
}

SpaceRect GObject::getBoundingBox() const
{
	return cpShapeGetBB(bodyShape);
}

SpaceVect GObject::getDimensions() const
{
	return getBoundingBox().dimensions;
}

SpaceFloat GObject::getMomentOfInertia() const
{
	return cpBodyGetMoment(body);
}

void GObject::updateParametricMove()
{
	SpaceFloat& t = parametric_t;
	auto f = parametric_f;

	if (t >= 0.0 && f) {
		setPos(f(t));
		timerIncrement(t);
	}
}

SpaceFloat GObject::getTraction() const
{
	return crntFloorCenterContact.isValid() ? crntFloorCenterContact.get()->getFrictionCoeff() : 1.0;
}

//END PHYSICS

//BEGIN GRAPHICS

GraphicsLayer GObject::sceneLayer() const {
	return GraphicsLayer::ground;
}

bool GObject::isGraphicsObject() const {
	return spriteID || drawNodeID || lightID;
}

int GObject::sceneLayerAsInt() const {
	return to_int(sceneLayer());
};

sprite_update GObject::updateSprite()
{
	bool visible = space->isInPlayerRoom(getPos());
	bool updateRequired = false;
	bool fadeOut = false;
	bool fadeIn = false;
	SpaceVect p = getPos();
	SpaceFloat a = getAngle();

	if (p != prevPos || a != prevAngle)
		updateRequired = true;

	if (!visible && !isInFade) {
		isInFade = true;
		fadeOut = true;
		updateRequired = true;
	}
	else if (visible && isInFade) {
		isInFade = false;
		fadeIn = true;
		updateRequired = true;
	}

	return sprite_update {
		spriteID,
		drawNodeID,
		lightID,
		toCocos(p) * app::pixelsPerTile,
		toCocosAngle(a),
		fadeIn,
		fadeOut,
		rotateSprite,
		updateRequired
	};
}

void GObject::initializeGraphics()
{
	sprite_properties _sprite = app::getSprite(getSprite());

	if (_sprite.filename.empty())
		return;

	float zoom = getSpriteZoom(_sprite, getRadius());
	string resPath = "sprites/" + _sprite.filename + ".png";
	
	rotateSprite = true;

	if (_sprite.size == make_pair(1, 1)) {
		spriteID = space->createSprite(
			&graphics_context::createSprite,
			resPath,
			sceneLayer(),
			getInitialCenterPix(),
			zoom
		);
	}
	else if (
		_sprite.size.first > 1 &&
		_sprite.size.second == 1 &&
		_sprite.duration > 0.0f
		) {
		spriteID = space->createSprite(
			&graphics_context::createLoopAnimation,
			_sprite.filename,
			_sprite.size.first,
			_sprite.duration,
			sceneLayer(),
			getInitialCenterPix(),
			zoom
		);
	}
	else {
		log("Invalid ImageSprite size %d,%d", _sprite.size.first, _sprite.size.second);
	}

	space->graphicsNodeAction(&Node::setRotation, spriteID, toCocosAngle(prevAngle));

	if (spriteID != 0 && _sprite.color != Color3B::BLACK && _sprite.color != Color3B::WHITE) {
		space->graphicsNodeAction(&Node::setColor, spriteID, _sprite.color);
	}
}

void GObject::initLightSource()
{
	auto source = getLightSource();
	if (source) {
		lightID = space->addLightSource(source);
		space->addGraphicsAction(&graphics_context::setLightSourcePosition, lightID, prevPos);
		space->addGraphicsAction(&graphics_context::setLightSourceAngle, lightID, prevAngle);
	}
}

void GObject::addGraphicsAction(ActionGeneratorType gen, SpriteID id )
{
	SpriteID actual = id == 0 ? spriteID : id;

	if (actual != 0) {
		space->addGraphicsAction(
			&graphics_context::runSpriteAction,
			actual,
			gen
		);
	}
}

void GObject::stopGraphicsAction(cocos_action_tag tag, SpriteID id)
{
	SpriteID actual = id == 0 ? spriteID : id;

	if (actual != 0) {
		space->addGraphicsAction(
			&graphics_context::stopSpriteAction,
			actual,
			tag
		);
	}
}

void GObject::setSpriteZoom(float zoom)
{
	if (spriteID != 0) {
		space->addGraphicsAction(&graphics_context::setSpriteZoom, spriteID, zoom);
	}
	else {
		log("GObject::setSpriteZoom: %s does not have a sprite!", getName());
	}
}

void GObject::setSpriteOpacity(unsigned char op)
{
	if (spriteID != 0) {
		space->graphicsNodeAction(&Node::setOpacity, spriteID, to_uchar(128));
	}
	else {
		log("GObject::setSpriteOpacity: %s does not have a sprite!", getName());
	}
}


//END GRAPHICS

//BEGIN AUDIO

ALuint GObject::playSoundSpatial(const string& path, float volume, bool loop, float yPos)
{
	ALuint soundSource = space->audioContext->playSoundSpatial(
		path,
		toVec3(getPos()),
		toVec3(getVel(), yPos),
		volume,
		loop
	);

	if (soundSource != 0) space->addSpatialSound(this, soundSource);
	return soundSource;
}

void GObject::stopSound(ALuint sourceID)
{
	space->audioContext->endSound(sourceID);
	space->removeSpatialSound(sourceID);
}

//END AUDIO

//BEGIN SPELLS

bool GObject::cast(shared_ptr<Spell> spell)
{
	if (crntSpell.get()) {
		crntSpell->end();
	}
	spell->init();
	crntSpell = spell;

	return true;
}

void GObject::stopSpell()
{
	if (crntSpell.get())
		crntSpell->end();
	crntSpell.reset();
}

void GObject::updateSpells()
{
	if (crntSpell.get()) {
		if (crntSpell->isActive()) {
			crntSpell->update();
		}
		else {
			crntSpell->end();
			crntSpell = nullptr;
		}
	}
}

void GObject::addMagicEffect(shared_ptr<MagicEffect> newEffect)
{
	magicEffectsToAdd.push_back(newEffect);
}

void GObject::updateMagicEffects()
{
	for (auto it = magicEffectsToAdd.begin(); it != magicEffectsToAdd.end(); ++it)
	{
		shared_ptr<MagicEffect> newEffect = *it;

		if (!newEffect || newEffect.get()->crntState != MagicEffect::state::created) {
			log("GObject::addMagicEffect: invalid magic effect");
			continue;
		}

		magicEffects.push_back(newEffect);
	}

	magicEffectsToAdd.clear();

	for (auto it = magicEffects.begin(); it != magicEffects.end(); ++it)
	{
		MagicEffect* _crntEffect = (*it).get();

		switch (_crntEffect->crntState)
		{
		case MagicEffect::state::created:
			_crntEffect->init();
			_crntEffect->crntState = MagicEffect::state::active;
			//intentional fall through - update will be first called on the same frame as init
		case MagicEffect::state::active:
			_crntEffect->update();
			break;
		case MagicEffect::state::ending:
			_crntEffect->end();
			_crntEffect->crntState = MagicEffect::state::expired;
			magicEffectsToRemove.push_back(*it);
			break;
		default:
			log("Invalid MagicEffect state %d", to_int(_crntEffect->crntState));
			break;
		}
	}

	for (auto it = magicEffectsToRemove.begin(); it != magicEffectsToRemove.end(); ++it)
	{
		magicEffects.remove(*it);
	}

}

//END SPELLS
