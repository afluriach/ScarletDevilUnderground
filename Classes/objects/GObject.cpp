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
#include "physics_context.hpp"
#include "sol_util.hpp"
#include "SpellDescriptor.hpp"
#include "value_map.hpp"

unordered_map<type_index, string> GObject::typeNameMap;

GObject::GObject(
	GSpace* space,
	ObjectIDType id,
	const object_params& params,
	const physics_params& phys,
	local_shared_ptr<object_properties> props
) :
	props(props),
    name(params.name),
	space(space),
	uuid(id),
	type(phys.type),
	layers(phys.layers),
	sensor(phys.sensor),
	mass(phys.mass),
	prevPos(params.pos),
	prevAngle(params.angle),
	active(params.active),
	hidden(params.hidden)
{
	dimensions = params.dimensions.isZero() ? 
		(props ? props->dimensions : SpaceVect::zero) : 
		params.dimensions;
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

bool GObject::conditionalLoad(GSpace* space, local_shared_ptr<object_properties> props, const object_params& params)
{
	if (params.name.size() > 0 && space->getAreaStats().isObjectRemoved(params.name) ) {
		return false;
	}

    auto objects = space->scriptVM->_state["objects"];
	auto cls = objects[props->clsName];

    if (cls.valid()) {
		sol::function f = cls["conditionalLoad"];

		if (f && !f(space, params, props)) {
			log0("object load canceled");
			return false;
		}
	}

	return true;
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
        log1("Unknown object type %s!", type.c_str());
        return nullptr;
    }
}

ObjectGeneratorType GObject::factoryMethodByType(const string& type, const ValueMap& args)
{
	return [type,args](GSpace* space, ObjectIDType id) -> GObject* {
		return constructByType(space, id, type, args);
	};
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

local_shared_ptr<object_properties> GObject::getProps() const {
    return props;
}

string GObject::getProperName() const {
    return props ? props->properName : "";
}

string GObject::getClsName() const {
    return props ? props->clsName : "";
}

gobject_ref GObject::getRef() const
{
	return gobject_ref(this);
}

string GObject::getName() const
{
	return name;
}

string GObject::getTypeIndexName() const
{
	return getOrDefault<type_index, string>(typeNameMap, typeid(*this), "GObject");
}

string GObject::toString() const
{
	stringstream s;
	string _cls = getClsName();
	string name = getName();
	s << getTypeIndexName() << ":" << uuid;

	if (_cls.size() > 0) {
		s << "(" << _cls << ")";
	}

	if (name.size() > 0) {
		s << "\"" << name << "\"";
	}

	return s.str();
}

//BEGIN LOGIC

void GObject::init()
{
	createLight();
}

void GObject::update()
{
    updateEffects();
	updateFloorSegment();
	updateParametricMove();
}

void GObject::onRemove()
{
    runMethodIfAvailable("onRemove");
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

    runMethodIfAvailable("onActivate");
}
void GObject::deactivate()
{
	active = false;

	runMethodIfAvailable("onDeactivate");
}

void GObject::toggleActive()
{
    if(active)
        deactivate();
    else
        activate();
}

void GObject::scriptInitialize()
{
    runMethodIfAvailable("initialize");
}

bool GObject::hasMethod(const string& name)
{
	if (!scriptObj) return false;

	return sol::hasMethod(scriptObj, name);
}

sol::object GObject::getScriptField(const string& name)
{
    if(!scriptObj)
        return sol::object();
        
    return scriptObj[name];
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
		obstacles,
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
	SpaceVect dimensions = attributes.getDimensions(props);

	return space->createBullet(
		Bullet::makeParams(getPos() + displacement, angle, velocity, angularVelocity, dimensions),
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

gobject_ref GObject::parametricBullet(
	local_shared_ptr<bullet_properties> props,
	SpaceVect displacement,
	parametric_space_function f,
	SpaceFloat angle,
	SpaceFloat angularVelocity,
	bool obstacleCheck
) {
	bullet_attributes attr = getBulletAttributes(props);
	attr.parametric = f;

	return _launchBullet(
		attr,
		props,
		displacement,
		angle,
		angularVelocity,
		obstacleCheck
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
	SpaceVect dimensions = attributes.getDimensions(props);
	SpaceFloat speed = attributes.getLaunchSpeed(props, angle);

	if (obstacleCheck && isBulletObstacle(position, dimensions.getMax()))
		return nullptr;

	return space->createBullet(
		Bullet::makeLaunchParams(position, angle, speed, angularVelocity, dimensions),
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

bool GObject::teleport(SpaceVect pos)
{
	bool success = !isObstacle(pos);

	if (success) {
		setPos(pos);
	}

	return success;
}

bool GObject::isObstacle(SpaceVect pos)
{
	return space->physicsContext->obstacleRadiusQuery(
		this,
		pos,
		getRadius(),
		obstacles,
		PhysicsLayers::ground
	);
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

	SpaceVect p = getPos();
	crntFloorCenterContact = space->floorPointQuery(p);

	if (crntFloorCenterContact) {
		SpaceFloat _uk = uk();
		if (_uk > 0.0) {
			updateFriction(_uk * crntFloorCenterContact->getTraction());
		}
	}
	//If not touching any floor, check for pitfall.
	else {
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

SpaceFloat GObject::uk() const {
	if (props)
		return props->friction;
	else
		return 0.0;
}

void GObject::initializeBody()
{
	if (dimensions.isZero())
	{
		log0("initializeBody: zero dimensions");
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

shared_ptr<sprite_properties> GObject::getSprite() const
{
	if (props)
		return props->sprite;
	else
		return nullptr;
}

shared_ptr<LightArea> GObject::getLightSource() const
{
	if (props)
		return props->light;
	else
		return nullptr;
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

        runMethodIfAvailable("spriteFadeOut");
	}
	else if (visible && isInFade) {
		isInFade = false;
		fadeIn = true;
		updateRequired = true;
  
        runMethodIfAvailable("spriteFadeIn");
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
	auto sprite = getSprite();
	if (!sprite)
		return;
	sprite_properties& _sprite = *sprite.get();

	if (_sprite.filename.empty())
		return;

	float zoom = getSpriteZoom(sprite, getRadius());
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
		log2("Invalid ImageSprite size %d,%d", _sprite.size.first, _sprite.size.second);
	}

	space->graphicsNodeAction(&Node::setRotation, spriteID, toCocosAngle(prevAngle));

	if (spriteID != 0 && _sprite.color != Color3B::BLACK && _sprite.color != Color3B::WHITE) {
		space->graphicsNodeAction(&Node::setColor, spriteID, _sprite.color);
	}
 
    runMethodIfAvailable("initializeGraphics");
}

void GObject::createLight()
{
	auto source = getLightSource();
	if (source && lightID == 0) {
		lightID = space->addLightSource(source, prevPos, prevAngle);
	}
}

void GObject::removeLight()
{
	if (lightID != 0) {
		space->removeLightSource(lightID);
        lightID = 0;
	}
}

void GObject::setLightSourceAngle(SpaceFloat angle)
{
    if(lightID != 0) {
        space->addGraphicsAction(
            &graphics_context::setLightSourceAngle,
            lightID,
            angle
        );
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
		log1("GObject::setSpriteZoom: %s does not have a sprite!", toString());
	}
}

void GObject::setSpriteOpacity(unsigned char op)
{
	if (spriteID != 0) {
		space->graphicsNodeAction(&Node::setOpacity, spriteID, to_uchar(op));
	}
	else {
		log1("GObject::setSpriteOpacity: %s does not have a sprite!", toString());
	}
}

void GObject::setSpriteVisible(bool val)
{
	if(spriteID != 0)
		space->graphicsNodeAction(&Node::setVisible, spriteID, val);
}

void GObject::setSpriteTexture(const string& texture)
{
	if (spriteID != 0) {
		space->addGraphicsAction(
			&graphics_context::setSpriteTexture,
			spriteID,
			texture
		);
	}
}

void createLight();
void removeLight();

//END GRAPHICS

//BEGIN AUDIO

ALuint GObject::playSoundSpatial(const string& path, float volume, bool loop, float yPos)
{
#if use_sound
	ALuint soundSource = space->audioContext->playSoundSpatial(
		path,
		toVec3(getPos()),
		toVec3(getVel(), yPos),
		volume,
		loop
	);

	if (soundSource != 0) space->addSpatialSound(this, soundSource);
	return soundSource;
#else
    return 0;
#endif
}

void GObject::stopSound(ALuint sourceID)
{
#if use_sound
	space->audioContext->endSound(sourceID);
	space->removeSpatialSound(sourceID);
#endif
}

//END AUDIO

//BEGIN SPELLS

local_shared_ptr<Spell> GObject::cast(const SpellDesc* desc)
{
	if (!desc) {
		log0("Null SpellDescriptor!");
		return nullptr;
	}
 
    if (!applyInitialSpellCost(desc->getCost())) {
		return nullptr;
	}
 
    local_shared_ptr<Spell> spell = desc->generate(this);
  
    if(!desc->getSFX().empty())
        playSoundSpatial("sfx/" + desc->getSFX() + ".wav");
        
    spell->start();
    
    if (logSpells) {
		log1("Spell %s (%u) created and initialized.", spell->getName());
	}

	return spell;
}

local_shared_ptr<MagicEffect> GObject::applyMagicEffect(const MagicEffectDescriptor* effect, effect_attributes attr)
{
    local_shared_ptr<MagicEffect> e;
    if (effect->canApply(this, attr)) {
		effect_params params = { this, effect->getFlags(), effect, attr };
		e = effect->generate(params);
        effects.push_back(e);
	}
    return e;
}

void GObject::updateEffect(MagicEffect* effect)
{
    if (effect->getState() == MagicEffect::state::created)
        effect->runInit();

    if(effect->getState() == MagicEffect::state::active)
        effect->runUpdate();
    
    if(effect->getState() == MagicEffect::state::ending)
        effect->runEnd();
}

void GObject::updateEffects()
{
    auto it = effects.begin();
    
    while(it != effects.end()){
        auto e = *it;
        
        if(e->getState() == MagicEffect::state::expired){
            it = effects.erase(it);
            continue;
        }
        
        updateEffect(e.get());
        
        if(e->getState() == MagicEffect::state::expired)
            it = effects.erase(it);
        else
            ++it;
    }
}

void GObject::removeEffects()
{
    auto it = effects.begin();
    while(it != effects.end()){
        auto e = *it;
        e->remove();
        
        if(e->getState() != MagicEffect::state::expired)
            e->end();
            
        it = effects.erase(it);
    }
}

//END SPELLS
