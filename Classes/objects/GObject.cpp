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
#include "AreaSensor.hpp"
#include "audio_context.hpp"
#include "Bullet.hpp"
#include "FloorSegment.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "LuaAPI.hpp"
#include "MagicEffect.hpp"
#include "MagicEffectSystem.hpp"
#include "physics_context.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "SpellSystem.hpp"
#include "value_map.hpp"

unordered_map<type_index, string> GObject::typeNameMap;

GObject::GObject(
	GSpace* space,
	ObjectIDType id,
	const object_params& params,
	const physics_params& phys
) :
	space(space),
	name(params.name),
	uuid(id),
	type(phys.type),
	layers(phys.layers),
	sensor(phys.sensor),
	dimensions(phys.dimensions),
	mass(phys.mass),
	prevPos(params.pos),
	prevAngle(params.angle),
	active(params.active),
	hidden(params.hidden)
{
	initializeBody();

	if (body) {
		setAngle(params.angle);
		setVel(params.vel);
		setAngularVel(params.angularVel);
	}
}

GObject::~GObject()
{
}

void GObject::removePhysicsObjects()
{
	space->physicsContext->removeObject(body);
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
	auto it1 = objectInfo.find(type);
	auto it2 = namedObjectTypes.find(type);

	if (it1 != objectInfo.end()) {
		AdapterType adapter = it1->second.consAdapter;
        return adapter(space, id, args);
    }
	else if (it2 != namedObjectTypes.end()) {
		AdapterType adapter = it2->second;
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

bool GObject::isValidObjectType(string typeName)
{
	auto it1 = objectInfo.find(typeName);
	auto it2 = namedObjectTypes.find(typeName);

	return it1 != objectInfo.end() || it2 != namedObjectTypes.end();
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

string GObject::getClsName() const
{
	return "";
}

//BEGIN LOGIC

void GObject::init()
{
	initLightSource();
	
	scriptInitialize();
}

void GObject::update()
{
	updateFloorSegment();
	
	updateParametricMove();
	updateFSM();
}

void GObject::onPitfall()
{
	space->removeObjectWithAnimation(this, pitfallShrinkAction().generator);
}

int GObject::getCrntRoomID() const {
	return crntRoom ? crntRoom->getID() : -1;
}

void GObject::setCrntRoom(RoomSensor* room)
{
	crntRoom = room;
}

void GObject::updateRoomQuery()
{
	GObject* result = space->physicsContext->pointQuery(
		getPos(),
		GType::areaSensor,
		PhysicsLayers::all
	);

	if (auto rs = dynamic_cast<RoomSensor*>(result)) {
		setCrntRoom(rs);
	}
}

void GObject::activate()
{
	active = true;

	if (hasMethod("onActivate")) {
		runVoidScriptMethod("onActivate");
	}
}
void GObject::deactivate()
{
	active = false;

	if (hasMethod("onDeactivate")) {
		runVoidScriptMethod("onDeactivate");
	}
}

void GObject::updateFSM() {
	if (fsm && !isFrozen)
		fsm->update();
}

local_shared_ptr<ai::Thread> GObject::addThread(local_shared_ptr<ai::Function> threadMain) {
	if (!fsm) {
		fsm = make_unique<ai::StateMachine>(this);
	}

	return fsm->addThread(threadMain);
}

void GObject::removeThread(local_shared_ptr<ai::Thread> t) {
	if (fsm) {
		fsm->removeThread(t);
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

void GObject::scriptInitialize()
{
	if (scriptObj) {
		sol::function f = scriptObj["initialize"];
		if (f) f(scriptObj);
	}
}

bool GObject::hasMethod(const string& name)
{
	if (!scriptObj) return false;

	sol::function f = scriptObj[name];
	return to_bool(f);
}

bool GObject::hit(DamageInfo damage, SpaceVect n)
{
	return false;
}

bullet_attributes GObject::getBulletAttributes(local_shared_ptr<bullet_properties> props) const
{
	return bullet_attributes::getDefault();
}

bool GObject::isBulletObstacle(SpaceVect pos, SpaceFloat radius)
{
	return space->physicsContext->obstacleRadiusQuery(
		this,
		pos,
		radius,
		bulletObstacles,
		PhysicsLayers::ground
	);
}

gobject_ref GObject::_spawnBullet(
	const bullet_attributes& attributes,
	local_shared_ptr<bullet_properties> props,
	SpaceVect displacement,
	SpaceVect velocity,
	SpaceFloat angle,
	SpaceFloat angularVelocity
) {
	return space->createBullet(
		Bullet::makeParams(getPos() + displacement, angle, velocity, angularVelocity),
		attributes,
		props
	);
}

gobject_ref GObject::spawnBullet(
	local_shared_ptr<bullet_properties> props,
	SpaceVect displacement,
	SpaceVect velocity,
	SpaceFloat angle,
	SpaceFloat angularVelocity
) {
	bullet_attributes attr = getBulletAttributes(props);

	return _spawnBullet(
		attr,
		props,
		displacement,
		velocity,
		angle,
		angularVelocity
	);
}

gobject_ref GObject::_launchBullet(
	const bullet_attributes& attributes,
	local_shared_ptr<bullet_properties> props,
	SpaceVect displacement,
	SpaceFloat angle,
	SpaceFloat angularVelocity,
	bool obstacleCheck
) {
	SpaceVect position = getPos() + displacement;

	if (obstacleCheck && isBulletObstacle(position, props->dimensions.getMax()))
		return nullptr;

	return space->createBullet(
		Bullet::makeParams(position, angle, SpaceVect::zero, angularVelocity),
		attributes,
		props
	);
}

gobject_ref GObject::launchBullet(
	local_shared_ptr<bullet_properties> props,
	SpaceVect displacement,
	SpaceFloat angle,
	SpaceFloat angularVelocity,
	bool obstacleCheck
) {
	return _launchBullet(
		getBulletAttributes(props),
		props,
		displacement,
		angle,
		angularVelocity,
		obstacleCheck
	);
}

//END LOGIC

//BEGIN PHYSICS

void GObject::launch() {
	setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));
}

void GObject::launchAtTarget(GObject* target)
{
	setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));
	setAngularVel(0.0f);
}

Vec2 GObject::getInitialCenterPix()
{
    SpaceVect centerPix(prevPos);
    centerPix *= app::pixelsPerTile;
        
    return toCocos(centerPix);
}

SpaceVect GObject::getPos() const {
    return body->GetPosition();
}

SpaceVect GObject::getDeltaPos() const {
	return getPos() - prevPos;
}

void GObject::setPos(SpaceVect p){
	if (space->isInCallback()) {
		space->addUpdateAction([this, p]()->void {
			body->SetPosition(toBox2D(p));
		});
	}
	else {
		body->SetPosition(toBox2D(p));
	}
}
    
void GObject::setAngle(SpaceFloat a){
	if (space->isInCallback()) {
		space->addUpdateAction([this,a]()->void {
			body->SetAngle(a - float_pi*0.5);
		});
	}
	else {
		body->SetAngle(a - float_pi*0.5);
	}
}
    
SpaceFloat GObject::getAngle() const {
	return canonicalAngle(body->GetAngle() + float_pi*0.5);
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
    return body->GetLinearVelocity();
}
    
void GObject::setVel(SpaceVect v) {
	body->SetLinearVelocity(toBox2D(v));
}

SpaceFloat GObject::getAngularVel() const{
    return body->GetAngularVelocity();
}
    
void GObject::setAngularVel(SpaceFloat w){
	body->SetAngularVelocity(w);
}

void GObject::applyForceForSingleFrame(SpaceVect f){
	body->ApplyLinearImpulseToCenter(toBox2D(f * app::params.secondsPerFrame), true);
}

void GObject::applyImpulse(SpaceVect i) {
	body->ApplyLinearImpulseToCenter(toBox2D(i), true);
}

void GObject::applyImpulse(SpaceFloat mag, SpaceFloat angle){
   applyImpulse(SpaceVect::ray(mag,angle));
}

void GObject::setParametricMove(parametric_space_function f, parametric_type move_type)
{
	parametricMotion = make_unique<parametric_motion>();

	parametricMotion->parametric_t = 0.0;
	parametricMotion->parametric_f = f;
	parametricMotion->parametric_move = move_type;
}

void GObject::removeParametricMove()
{
	parametricMotion.reset();
}

PhysicsLayers GObject::getCrntLayers() const
{
	return static_cast<PhysicsLayers>(bodyShape->GetLayers());
}

void GObject::setLayers(PhysicsLayers layers)
{
	bodyShape->SetLayers(to_uint(layers));

	if ( !bitwise_and_bool(PhysicsLayers::floor, layers) ) {
		crntFloorCenterContact = nullptr;
		crntFloorContacts.clear();
	}
}

bool GObject::isOnFloor() const
{
	return bitwise_and(PhysicsLayers, getCrntLayers(), PhysicsLayers::floor) != PhysicsLayers::none;
}

SpaceVect GObject::getFloorVelocity() const
{
	return (crntFloorCenterContact && isOnFloor()) ? crntFloorCenterContact->getVel() : SpaceVect::zero;
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
		SpaceVect local = p - ref->getPos();
		SpaceVect dim = ref->getDimensions();
		if (abs(local.x) <= dim.x / 2.0 && abs(local.y) <= dim.y / 2.0) {
			crntFloorCenterContact = ref;
			break;
		}
	}

	if (crntFloorCenterContact) {
		SpaceFloat _uk = uk();
		if (_uk > 0.0) {
			updateFriction(_uk * crntFloorCenterContact->getTraction());
		}
	}
	//If not touching any floor, check for pitfall.
	else if(crntFloorContacts.empty()){
		Pitfall* pitfall = space->pitfallPointQuery(p);
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

void GObject::onContactFloorSegment(FloorSegment* fs)
{
	if (fs && isOnFloor())
	{
		if (crntFloorContacts.find(fs) != crntFloorContacts.end()) {
			log("onContactFloorSegment duplicate add attempted for floor ID %d", fs->uuid);
			return;
		}

		fs->onContact(this);
		crntFloorContacts.insert(fs);
	}
}

void GObject::onEndContactFloorSegment(FloorSegment* fs)
{
	if (fs && isOnFloor())
	{
		if (crntFloorContacts.find(fs) == crntFloorContacts.end()) {
			log("onEndContactFloorSegment floor ID %d not found", fs->uuid);
			return;
		}

		fs->onEndContact(this);
		crntFloorContacts.erase(fs);
	}
}

SpaceRect GObject::getBoundingBox() const
{
	return SpaceRect(
		getPos(),
		dimensions.x == 0 ? SpaceVect(dimensions.x, dimensions.x) : dimensions
	);
}

SpaceVect GObject::getDimensions() const
{
	return dimensions;
}

SpaceFloat GObject::getMomentOfInertia() const
{
	return body->GetInertia();
}

SpaceFloat GObject::getRadius() const
{
	return dimensions.getMax();
}

void GObject::initializeBody()
{
	if (dimensions.isZero())
	{
		log("initializeBody: zero dimensions");
		return;
	}
	else if(dimensions.y > 0.0)
	{
		tie(body, bodyShape) = space->physicsContext->createRectangleBody(
			prevPos,
			dimensions,
			getMass(),
			getFullType(),
			getLayers(),
			getSensor(),
			make_any<GObject*>(this)
		);
	}
	else
	{
		tie(body, bodyShape) = space->physicsContext->createCircleBody(
			prevPos,
			dimensions.x,
			getMass(),
			getFullType(),
			getLayers(),
			getSensor(),
			make_any<GObject*>(this)
		);
	}
}

void GObject::updateParametricMove()
{
	if (!parametricMotion)
		return;

	SpaceFloat& t = parametricMotion->parametric_t;
	auto f = parametricMotion->parametric_f;

	if (t >= 0.0 && f) {
		switch (parametricMotion->parametric_move)
		{
		case parametric_type::position:
			setPos(f(t));
		break;
		case parametric_type::velocity:
			setVel(f(t));
		break;
		default:
		break;
		}
		timerIncrement(t);
	}
}

SpaceFloat GObject::getTraction() const
{
	return (crntFloorCenterContact && isOnFloor()) ? crntFloorCenterContact->getTraction() : 1.0;
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
	bool visible = space->isInPlayerRoom(getCrntRoomID());
	bool updateRequired = false;
	bool fadeOut = false;
	bool fadeIn = false;
	SpaceVect p = getPos();
	SpaceFloat a = getAngle();

	if (p != prevPos || a != prevAngle) {
		updateRequired = true;

		prevPos = p;
		prevAngle = a;
	}

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
		lightID = space->addLightSource(source, prevPos, prevAngle);
	}
}

void GObject::addGraphicsAction(GraphicsAction action)
{
	addGraphicsAction(action, spriteID);
}

void GObject::addGraphicsAction(GraphicsAction action, SpriteID id )
{
	if (id != 0) {
		space->addGraphicsAction(
			&graphics_context::runSpriteAction,
			id,
			action.generator
		);
	}
}

void GObject::stopGraphicsAction(cocos_action_tag tag)
{
	stopGraphicsAction(tag, spriteID);
}

void GObject::stopGraphicsAction(cocos_action_tag tag, SpriteID id)
{
	if (id != 0) {
		space->addGraphicsAction(
			&graphics_context::stopSpriteAction,
			id,
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
		space->graphicsNodeAction(&Node::setOpacity, spriteID, to_uchar(op));
	}
	else {
		log("GObject::setSpriteOpacity: %s does not have a sprite!", getName());
	}
}

void GObject::setSpriteVisible(bool val)
{
	if(spriteID != 0)
		space->graphicsNodeAction(&Node::setVisible, spriteID, val);
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

unsigned int GObject::cast(local_shared_ptr<SpellDesc> desc)
{
	return space->spellSystem->cast(desc, this);
}

unsigned int GObject::applyMagicEffect(local_shared_ptr<MagicEffectDescriptor> effect, float magnitude, float length)
{
	return space->magicEffectSystem->applyEffect(this, effect, magnitude, length);
}

//END SPELLS
