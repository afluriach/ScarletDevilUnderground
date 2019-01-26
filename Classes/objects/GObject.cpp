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
#include "GObject.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "LuaAPI.hpp"
#include "macros.h"
#include "MagicEffect.hpp"
#include "scenes.h"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "util.h"
#include "value_map.hpp"

const float GObject::objectFadeOutTime = 1.5f;
const float GObject::objectFadeInTime = 0.5f;
const GLubyte GObject::objectFadeOpacity = 0;

GObject::GObject(GSpace* space, ObjectIDType uuid, const ValueMap& args) :
GObject(space, uuid, args, false)
{
}

GObject::GObject(GSpace* space, ObjectIDType uuid, const ValueMap& obj, bool anonymous) :
	space(space),
	name(!anonymous ? obj.at("name").asString() : ""),
	uuid(uuid),
	anonymous(anonymous)
{
	//Interpret coordinates as center, unit space.
	initialCenter = SpaceVect(getFloat(obj, "pos_x"), getFloat(obj, "pos_y"));

	if (obj.find("vel_x") != obj.end() && obj.find("vel_y") != obj.end()) {
		setInitialVelocity(SpaceVect(getFloat(obj, "vel_x"), getFloat(obj, "vel_y")));
	}
#if GOBJECT_LUA
	if (!anonymous) {
		ctx = make_unique<Lua::Inst>(boost::lexical_cast<string>(uuid) + "_" + name);
	}
#endif

    if(!anonymous && logCreateObjects)
        log("%s created at %.1f,%.1f.", name.c_str(),initialCenter.x, initialCenter.y);

	setInitialAngle(float_pi / 2.0f);
}

GObject::GObject(GSpace* space, ObjectIDType uuid, const string& name, const SpaceVect& pos, SpaceFloat angle, bool anonymous) :
	space(space),
	name(name),
    anonymous(anonymous),
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
}

GObject::GObject(GSpace* space, ObjectIDType uuid, const string& name, const SpaceVect& pos, bool anonymous) :
	GObject(space, uuid, name, pos, float_pi / 2.0f, anonymous)
{}

GObject::~GObject()
{
}

GObject* GObject::constructByType(GSpace* space, ObjectIDType id, const string& type, const ValueMap& args )
{
    auto it = adapters.find(type);
    
    if(it != adapters.end()){
        AdapterType adapter =  it->second;
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
#if GOBJECT_LUA
	setupLuaContext();
	runLuaInit();
#endif
}

void GObject::update()
{
	multiUpdate();
	updateMessages();
#if GOBJECT_LUA
	runLuaUpdate();
#endif
	updateSprite();
	updateRadarPos();
	updateSpells();
	updateMagicEffects();
	updateFloorSegment();
}

void GObject::updateMessages()
{
	messages();
	messages.clear();
}

void GObject::onPitfall()
{
	space->removeObjectWithAnimation(this, pitfallShrinkAction());
}

//BEGIN PHYSICS

void GObject::setInitialVelocity(const SpaceVect&& v){
	multiInit += [=]() -> void { cpBodySetVel(body, v); };
}

void GObject::setInitialAngle(SpaceFloat a){
	multiInit += [=]() -> void { cpBodySetAngle(body, a); };
}

void GObject::setInitialAngularVelocity(SpaceFloat w){
    multiInit += [=]() -> void{ cpBodySetAngVel(body, w);};
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
	cpBodySetAngle(body, a);
}
    
SpaceFloat GObject::getAngle() const {
    return cpBodyGetAngle(body);
}

void GObject::rotate(SpaceFloat a){
    setAngle(canonicalAngle(getAngle() + a) );
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
    
void GObject::applyImpulse(SpaceFloat mag, SpaceFloat angle){
    SpaceVect v = SpaceVect::ray(mag,angle);
	cpBodyApplyImpulse(body, v, SpaceVect::zero);
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

void GObject::updateFloorSegment()
{
	if (getMass() <= 0.0 || !isOnFloor() || dynamic_cast<FloorSegment*>(this)) {
		return;
	}

	crntFloorCenterContact = nullptr;

	for (auto ref : crntFloorContacts)
	{
		SpaceVect local = cpBodyWorld2Local(ref.get()->body, getPos());
		SpaceVect dim = ref.get()->getDimensions();
		if (abs(local.x) <= dim.x / 2.0 && abs(local.y) <= dim.y / 2.0) {
			crntFloorCenterContact = ref;
			break;
		}
	}

	//If not on floor(s), use point query to detect belowFloor segment.
	if (!crntFloorCenterContact.isValid())
	{
		crntFloorCenterContact = space->floorSegmentPointQuery(getPos());
	}

	if (crntFloorContacts.size() == 0 && crntFloorCenterContact.isValid()) {
		crntFloorCenterContact.get()->exclusiveFloorEffect(this);
	}
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
	bool visible = space->isInCameraArea(getBoundingBox()) &&
		space->isInPlayerRoom(getPos());

    if(spriteID != 0){
		space->setSpritePosition(spriteID, toCocos(getPos())*App::pixelsPerTile);

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
	if (drawNodeID != 0 && visible && !isInFade)
	{
		space->setSpritePosition(drawNodeID, toCocos(getPos())*App::pixelsPerTile);
	}
}

//END GRAPHICS

//BEGIN SPELLS

void GObject::cast(shared_ptr<Spell> spell)
{
	if (crntSpell.get()) {
		crntSpell->end();
	}
	spell->init();
	crntSpell = spell;
}

void GObject::cast(const string& name)
{
	auto it_adaptor = Spell::spellDescriptors.find(name);

	if (it_adaptor != Spell::spellDescriptors.end()) {
		//Check for a Spell class
		cast(it_adaptor->second->generate(this));
		return;
	}

	log("Spell %s not available.", name.c_str());
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
