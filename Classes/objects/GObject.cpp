//
//  GObject.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "enum.h"
#include "FloorSegment.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "LuaAPI.hpp"
#include "macros.h"
#include "MagicEffect.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "util.h"
#include "value_map.hpp"

const float GObject::objectFadeOutTime = 1.5f;
const float GObject::objectFadeInTime = 0.5f;
const GLubyte GObject::objectFadeOpacity = 0;

GObject::GObject(GSpace* space, ObjectIDType uuid, const ValueMap& obj, bool anonymous) :
	space(space),
	name(!anonymous ? obj.at("name").asString() : ""),
	uuid(uuid),
	hidden(getBoolOrDefault(obj,"hidden", false)),
	anonymous(anonymous)
{
	//Interpret coordinates as center, unit space.
	initialCenter = getObjectPos(obj);
	prevPos = initialCenter;

#if GOBJECT_LUA
	if (!anonymous) {
		ctx = make_unique<Lua::Inst>(boost::lexical_cast<string>(uuid) + "_" + name);
	}
#endif

    if(!anonymous && logCreateObjects)
        log("%s created at %.1f,%.1f.", name.c_str(),initialCenter.x, initialCenter.y);
}

GObject::GObject(GSpace* space, ObjectIDType uuid, const string& name, const SpaceVect& pos, SpaceFloat angle) :
	space(space),
	name(name),
    anonymous(name.empty()),
	uuid(uuid),
	initialCenter(pos)
{
#if GOBJECT_LUA
    if(!anonymous){
        ctx = make_unique<Lua::Inst>(boost::lexical_cast<string>(uuid) + "_" + name);
    }
#endif

    if(logCreateObjects && !anonymous)
        log("%s created at %.1f,%.1f.", name.c_str(),initialCenter.x, initialCenter.y);

	setInitialAngle(angle);
	prevAngle = angle;
}

GObject::~GObject()
{
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

GObject::GeneratorType GObject::factoryMethodByType(const string& type, const ValueMap& args)
{
	return [type,args](GSpace* space, ObjectIDType id) -> GObject* {
		return constructByType(space, id, type, args);
	};
}


void GObject::init()
{
	multiInit();
	multiInit.clear();
#if GOBJECT_LUA
	setupLuaContext();
	runLuaInit();
#endif
}

void GObject::update()
{
	updateMessages();
#if GOBJECT_LUA
	runLuaUpdate();
#endif
	updateSprite();
	updateRadarPos();
	updateLight();
	updateSpells();
	updateMagicEffects();
	updateFloorSegment();

	prevPos = getPos();
	prevAngle = getAngle();
}

void GObject::updateMessages()
{
	for (auto f : messages) f();
	messages.clear();
}

void GObject::onPitfall()
{
	space->removeObjectWithAnimation(this, pitfallShrinkAction());
}

//BEGIN PHYSICS

void GObject::launch() {
	setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));
}

void GObject::setInitialVelocity(const SpaceVect& v){
	multiInit += [=]() -> void { cpBodySetVel(body, v); };
}

void GObject::setInitialAngle(SpaceFloat a){
	multiInit += [=]() -> void { cpBodySetAngle(body, a); };
}

void GObject::setInitialAngularVelocity(SpaceFloat w){
    multiInit += [=]() -> void{ cpBodySetAngVel(body, w);};
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
    SpaceVect centerPix(initialCenter);
    centerPix *= App::pixelsPerTile;
        
    return toCocos(centerPix);
}

SpaceVect GObject::getPos() const {
    return cpBodyGetPos(body);
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
	cpBodyApplyImpulse(body, f * App::secondsPerFrame, SpaceVect::zero);
}

void GObject::applyImpulse(SpaceVect i) {
	cpBodyApplyImpulse(body, i, SpaceVect::zero);
}

void GObject::applyImpulse(SpaceFloat mag, SpaceFloat angle){
   applyImpulse(SpaceVect::ray(mag,angle));
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

	//If not on floor(s), use point query to detect belowFloor segment.
	if (!crntFloorCenterContact.isValid())
	{
		crntFloorCenterContact = space->floorSegmentPointQuery(p);
	}

	if (crntFloorContacts.size() == 0 && crntFloorCenterContact.isValid()) {
		crntFloorCenterContact.get()->exclusiveFloorEffect(this);
	}

	if (crntFloorCenterContact.isValid() && uk() > 0.0) {
		updateFriction(uk() * crntFloorCenterContact.get()->getFrictionCoeff());
	}
}

void GObject::updateFriction(float frictionCoeff)
{
	//linear
	SpaceVect vel = getVel();
	SpaceFloat force = getMass() * App::Gaccel * frictionCoeff;

	//if acceleraion, dv/dt, or change in velocity over one frame is greater
	//than current velocity, apply stop instead
	if (App::Gaccel * frictionCoeff * App::secondsPerFrame < vel.length())
		applyForceForSingleFrame(vel * -force);
	else
		setVel(SpaceVect::zero);

	//rotational
	SpaceFloat angularVel = getAngularVel();
	SpaceFloat angularImpulse = getMomentOfInertia() * App::Gaccel *  frictionCoeff * App::secondsPerFrame;

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

void GObject::updateRadarPos()
{
	if (radar && body)
		cpBodySetPos(radar, cpBodyGetPos(body));
}

void GObject::updateLight()
{
	SpaceVect pos = getPos();
	SpaceFloat a = getAngle();
	if (lightID != 0) {
		if(pos != prevPos)
			space->setLightSourcePosition(lightID, pos);
		if(a != prevAngle)
			space->setLightSourceAngle(lightID, a);
	}
}

SpaceFloat GObject::getTraction() const
{
	return crntFloorCenterContact.isValid() ? crntFloorCenterContact.get()->getFrictionCoeff() : 1.0;
}

//END PHYSICS

//BEGIN LUA

#if GOBJECT_LUA

string GObject::getScriptVal(string name) {
    if_lua_ctx { return ctx->getSerialized(name); }
	else return "";
}

void GObject::setScriptVal(string field, string val) {
    if_lua_ctx { ctx->setSerialized(field, val); }
}

string GObject::_callScriptVal(string field, string args) {
    if_lua_ctx { return ctx->callSerialized(field, args); }
	else return "";
}

void GObject::runLuaInit() {
    if_lua_ctx { ctx->callIfExistsNoReturn("init"); }
}

void GObject::runLuaUpdate() {
    if_lua_ctx { ctx->callIfExistsNoReturn("update"); }
}

void GObject::setupLuaContext()
{
    if(anonymous)
        return;

	//Push this as a global variable in the object's script context.
	ctx->setGlobal(Lua::convert<GObject*>::convertToLua(this, ctx->state), "this");

	string scriptName = getScriptName();
	string scriptPath = "scripts/entities/" + scriptName + ".lua";
	if (scriptName != "" && FileUtils::getInstance()->isFileExist(scriptPath))
	{
		ctx->runFile(scriptPath);
	}
}

#endif

//END LUA

//BEGIN GRAPHICS

GraphicsLayer GObject::sceneLayer() const {
	return GraphicsLayer::ground;
}

int GObject::sceneLayerAsInt() const {
	return to_int(sceneLayer());
};

float GObject::zoom() const {
	return 1.0f;
}

void GObject::updateSprite()
{
	bool visible = space->isInPlayerRoom(getPos());
	SpaceVect p = getPos();
	SpaceFloat a = getAngle();

    if(spriteID != 0){

		if(p != prevPos)
			space->setSpritePosition(spriteID, toCocos(p)*App::pixelsPerTile);

		if (a != prevAngle && rotateSprite)
			space->setSpriteAngle(spriteID, toCocosAngle(a));

		if (!visible && !isInFade) {
			space->stopSpriteAction(spriteID, cocos_action_tag::object_fade);
			space->runSpriteAction(spriteID, objectFadeOut(objectFadeOutTime, objectFadeOpacity));
			isInFade = true;
		}
		else if (visible && isInFade) {
			space->stopSpriteAction(spriteID, cocos_action_tag::object_fade);
			space->runSpriteAction(spriteID, objectFadeOut(objectFadeInTime, 255));

			isInFade = false;
		}
    }
	if (drawNodeID != 0 && visible && !isInFade && p != prevPos)
	{
		space->setSpritePosition(drawNodeID, toCocos(p)*App::pixelsPerTile);
	}
}

//END GRAPHICS

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
			log("Invalid MagicEffect state %d", _crntEffect->crntState);
			break;
		}
	}

	for (auto it = magicEffectsToRemove.begin(); it != magicEffectsToRemove.end(); ++it)
	{
		magicEffects.remove(*it);
	}

}

//END SPELLS
