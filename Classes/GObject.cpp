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
#include "util.h"

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
}

GObject::GObject(const string& name, const SpaceVect& pos) :
	name(name),
	uuid(nextUUID++),
	ctx(make_unique<Lua::Inst>(boost::lexical_cast<string>(uuid) + "_" + name)),
	initialCenter(pos)
{
    if(logCreateObjects)
        log("%s created at %.1f,%.1f.", name.c_str(),initialCenter.x, initialCenter.y);
}

GObject::~GObject() {}

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

//BEGIN PHYSICS

     void GObject::setInitialVelocity(const SpaceVect&& v){
        multiInit += [=]() -> void{ this->body->setVel(v);};
    }

    Vec2 GObject::getInitialCenterPix()
    {
        SpaceVect centerPix(initialCenter);
        centerPix *= App::pixelsPerTile;
        
        return toCocos(centerPix);
    }

     SpaceVect GObject::getPos(){
        return body->getPos();
    }

     void GObject::setPos(float x, float y){
        body->setPos(SpaceVect(x,y));
    }
    
    void GObject::setAngle(float a){
        if(!body){
            log("GObject::setAngle: %s has no physics body!", name.c_str());
            return;
        }
        body->setAngle(a);
    }
    
     float GObject::getAngle(){
        if(!body){
            log("GObject::getAngle: %s has no physics body!", name.c_str());
            return 0.0;
        }
        return body->getAngle();
    }
    
     void GObject::rotate(float a){
        setAngle(canonicalAngle(getAngle() + a) );
    }
    
     SpaceVect GObject::getFacingVector(){
        return SpaceVect::ray(1.0f, getAngle());
    }
    
    void GObject::setDirection(Direction d) {
        if(body && d != Direction::none)
            body->setAngle(dirToPhysicsAngle(d));
    }
    
     SpaceVect GObject::getVel(){
        return body->getVel();
    }
    
     void GObject::setVel(SpaceVect v){
        body->setVel(v);
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
    return ctx->getSerialized(name);
}

void GObject::setScriptVal(string field, string val) {
    ctx->setSerialized(field, val);
}

string GObject::_callScriptVal(string field, string args) {
    return ctx->callSerialized(field, args);
}

void GObject::runLuaInit() {
    ctx->callIfExistsNoReturn("init");
}

void GObject::runLuaUpdate() {
    ctx->callIfExistsNoReturn("update");
}

void GObject::setupLuaContext()
{
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
