//
//  GObject.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "GObject.hpp"
#include "LuaAPI.hpp"
#include "macros.h"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "util.h"
#include "value_map.hpp"

unsigned int GObject::nextUUID = 1;

GObject::GObject(const ValueMap& obj) :
	name(obj.at("name").asString()),
	uuid(nextUUID++),
	ctx(make_unique<Lua::Inst>(boost::lexical_cast<string>(uuid) + "_" + name))
{
    //Interpret coordinates as center, unit space.
    initialCenter = SpaceVect(getFloat(obj, "pos_x"), getFloat(obj, "pos_y"));
    
    if(obj.find("vel_x") != obj.end() && obj.find("vel_y") != obj.end()){
        setInitialVelocity(SpaceVect(getFloat(obj, "vel_x"), getFloat(obj, "vel_y")));
    }
    
    if(logCreateObjects)
        log("%s created at %.1f,%.1f.", name.c_str(),initialCenter.x, initialCenter.y);

	setInitialAngle(float_pi / 2.0f);
}

GObject::GObject(const string& name, const SpaceVect& pos, float angle, bool anonymous) :
	name(name),
    anonymous(anonymous),
	uuid(nextUUID++),
	initialCenter(pos)
{
    if(!anonymous){
        ctx = make_unique<Lua::Inst>(boost::lexical_cast<string>(uuid) + "_" + name);
    }

    if(logCreateObjects && !anonymous)
        log("%s created at %.1f,%.1f.", name.c_str(),initialCenter.x, initialCenter.y);

	setInitialAngle(angle);
}

GObject::GObject(const string& name, const SpaceVect& pos, bool anonymous) :
	GObject(name, pos, float_pi / 2.0f, anonymous)
{}

GObject::~GObject()
{
}

GObject* GObject::constructByType(const string& type, const ValueMap& args )
{
    auto it = adapters.find(type);
    
    if(it != adapters.end()){
        AdapterType adapter =  it->second;
        return adapter(args);
    }
    else{
        log("Unknown object type %s!", type.c_str());
        return nullptr;
    }
}

void GObject::init()
{
	multiInit();
	setupLuaContext();
	runLuaInit();
}

void GObject::update()
{
	multiUpdate();
	runLuaUpdate();
	updateSprite();
	updateSpells();
}

//BEGIN PHYSICS

     void GObject::setInitialVelocity(const SpaceVect&& v){
        multiInit += [=]() -> void{ this->body->setVel(v);};
    }

    void GObject::setInitialAngle(float a){
        multiInit += [=]() -> void{ this->body->setAngle(a);};
    }

    void GObject::setInitialAngularVelocity(float w){
        multiInit += [=]() -> void{ this->body->setAngularVel(w);};
    }

    Vec2 GObject::getInitialCenterPix()
    {
        SpaceVect centerPix(initialCenter);
        centerPix *= App::pixelsPerTile;
        
        return toCocos(centerPix);
    }

	void GObject::applyPhysicsProperties()
	{
		if (physicsPropertiesToApply.setPos)
			body->setPos(physicsPropertiesToApply.pos);

		if (physicsPropertiesToApply.setAngle)
			body->setAngle(physicsPropertiesToApply.angle);

		if(physicsPropertiesToApply.setVel)
			body->setVel(physicsPropertiesToApply.vel);

		if (physicsPropertiesToApply.setAngularVel)
			body->setAngularVel(physicsPropertiesToApply.angularVel);

		physicsPropertiesToApply.setPos = false;
		physicsPropertiesToApply.setAngle = false;
		physicsPropertiesToApply.setVel = false;
		physicsPropertiesToApply.setAngularVel = false;
	}

     SpaceVect GObject::getPos() const {
        return body->getPos();
    }

     void GObject::setPos(float x, float y){
		 if (physicsPropertiesToApply.setPos) {
			 log(
				 "%s has multi-set position, from %f,%f to %f,%f",
				 name.c_str(),
				 physicsPropertiesToApply.pos.x,
				 physicsPropertiesToApply.pos.y,
				 x,
				 y
			 );

		 }
		 physicsPropertiesToApply.setPos = true;
         physicsPropertiesToApply.pos = SpaceVect(x,y);
    }
    
    void GObject::setAngle(float a){
		if (physicsPropertiesToApply.setAngle) {
			log(
				"%s has multi-set angle, from %f to %f",
				name.c_str(),
				physicsPropertiesToApply.angle,
				a
			);
		}

		physicsPropertiesToApply.setAngle = true;
		physicsPropertiesToApply.angle = a;
    }
    
     float GObject::getAngle() const {
        if(!body){
            log("GObject::getAngle: %s has no physics body!", name.c_str());
            return 0.0;
        }
        return canonicalAngle(body->getAngle());
    }
    
     void GObject::rotate(float a){
        setAngle(canonicalAngle(getAngle() + a) );
    }
    
     SpaceVect GObject::getFacingVector() const{
        return SpaceVect::ray(1.0f, getAngle());
    }
    
    void GObject::setDirection(Direction d) {
        if(body && d != Direction::none)
            setAngle(dirToPhysicsAngle(d));
    }
    
     SpaceVect GObject::getVel() const {
        return body->getVel();
    }
    
	 void GObject::setVel(SpaceVect v) {
		 if (physicsPropertiesToApply.setVel) {
			log(
				"%s has multi-set velocity, from %f,%f to %f,%f",
				name.c_str(),
				physicsPropertiesToApply.vel.x,
				physicsPropertiesToApply.vel.y,
				v.x,
				v.y
			);
		}

		 physicsPropertiesToApply.setVel = true;
		 physicsPropertiesToApply.vel = v;
    }

    float GObject::getAngularVel() const{
        return body->getAngularVel();
    }
    
     void GObject::setAngularVel(float w){
		 if (physicsPropertiesToApply.setAngularVel) {
			 log(
				 "%s has multi-set angular velocity, from %f to %f",
				 name.c_str(),
				 physicsPropertiesToApply.angularVel,
				 w
			 );
		 }

		 physicsPropertiesToApply.setAngularVel = true;
		 physicsPropertiesToApply.angularVel = w;
    }

     void GObject::applyForceForSingleFrame(SpaceVect f){
        body->applyImpulse(f * App::secondsPerFrame);
    }
    
     void GObject::applyImpulse(float mag, float angle){
        SpaceVect v = SpaceVect::ray(mag,angle);
        
        body->applyImpulse(v);
    }


//END PHYSICS

//BEGIN LUA

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

//END LUA

//BEGIN GRAPHICS

void GObject::updateSprite()
{
    if(sprite != nullptr){
        sprite->setPosition(toCocos(body->getPos())*App::pixelsPerTile);
    }
}

//END GRAPHICS

//BEGIN SPELLS

void GObject::cast(unique_ptr<Spell> spell)
{
	if (crntSpell.get()) {
		crntSpell->end();
	}
	spell->init();
	crntSpell = move(spell);
}

void GObject::cast(const string& name, const ValueMap& args)
{
	auto it_adaptor = Spell::spellDescriptors.find(name);

	if (it_adaptor != Spell::spellDescriptors.end()) {
		//Check for a Spell class
		cast(it_adaptor->second->generate(this, args));
		return;
	}

	log("Spell %s not available.", name.c_str());
}

void GObject::castByName(string name, const ValueMap& args)
{
	cast(name, args);
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

//END SPELLS
