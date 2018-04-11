//
//  GObject.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

unsigned int GObject::nextUUID = 1;

GObject::GObject(const ValueMap& obj) :
	name(obj.at("name").asString()),
	uuid(nextUUID++),
	ctx(boost::lexical_cast<string>(uuid) + "_" + name)
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
	ctx(boost::lexical_cast<string>(uuid) + "_" + name),
	initialCenter(pos)
{
    if(logCreateObjects)
        log("%s created at %.1f,%.1f.", name.c_str(),initialCenter.x, initialCenter.y);
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

void GObject::setupLuaContext()
{
	//Push this as a global variable in the object's script context.
	ctx.setGlobal(Lua::convert<GObject*>::convertToLua(this, ctx.state), "this");

	string scriptName = getScriptName();
	string scriptPath = "scripts/entities/" + scriptName + ".lua";
	if (scriptName != "" && FileUtils::getInstance()->isFileExist(scriptPath))
	{
		ctx.runFile(scriptPath);
	}
}

void SpriteObject::update()
{
    if(sprite != nullptr){
        sprite->setPosition(toCocos(body->getPos())*App::pixelsPerTile);
    }
}

void LoopAnimationSprite::initializeGraphics(Layer* layer)
{
    anim = TimedLoopAnimation::create();
    anim->loadAnimation(animationName(), animationSize(), animationDuration());
    
    layer->positionAndAddNode(anim, sceneLayerAsInt(), getInitialCenterPix(), zoom());
    sprite = anim;
}

PatchConSprite::PatchConSprite(const ValueMap& args) :
RegisterInit<PatchConSprite>(this),
RegisterUpdate<PatchConSprite>(this)
{
    auto it = args.find("direction");
    if(it != args.end()){
        Direction dir = stringToDirection(it->second.asString());
        if(dir != Direction::none)
            startingDirection = dir;
    }
}

void PatchConSprite::init()
{
    setDirection(startingDirection);
}

void PatchConSprite::initializeGraphics(Layer* layer)
{
    animSprite = PatchConAnimation::create();
    animSprite->loadAnimation(imageSpritePath());
    layer->positionAndAddNode(animSprite, sceneLayerAsInt(), getInitialCenterPix(), zoom());
    sprite = animSprite;
}

void PatchConSprite::update()
{
    SpaceVect dist = body->getVel()*App::secondsPerFrame;
    
    animSprite->accumulate(dist.length());
}

void PatchConSprite::setDirection(Direction d)
{
    GObject::setDirection(d);
    if(d == Direction::none) return;

    animSprite->setDirection(d);
}

Vec2 GObject::getInitialCenterPix()
{
    SpaceVect centerPix(initialCenter);
    centerPix *= App::pixelsPerTile;
    
    return toCocos(centerPix);
}

SpaceVect RectangleMapBody::getDimensionsFromMap(const ValueMap& arg)
{
    return SpaceVect(getFloat(arg, "dim_x"), getFloat(arg, "dim_y"));
}

void ImageSprite::loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest)
{
    Vec2 centerPix = getInitialCenterPix();
    sprite = ::loadImageSprite(resPath,sceneLayer,dest, centerPix, zoom());
    
    if(sprite == nullptr)
        log("%s sprite not loaded", name.c_str());
    else if(App::logSprites)
        log("%s sprite %s added at %.1f,%.1f, layer %d", name.c_str(), resPath.c_str(), expand_vector2(centerPix), sceneLayer);
}


void Spellcaster::cast(shared_ptr<Spell> spell)
{
    if(crntSpell.get()){
        crntSpell->end();
    }
    spell->init();
    crntSpell = spell;
}

void Spellcaster::cast(const string& name, const ValueMap& args)
{
    auto it_adaptor = Spell::adapters.find(name);
    
    if(it_adaptor != Spell::adapters.end()){
        //Check for a Spell class
        cast(it_adaptor->second(this, args));
        return;
    }
    auto it_script = Spell::scripts.find(name);
    if(it_script != Spell::scripts.end()){
        //Check for a spell script.
        cast(make_shared<ScriptedSpell>(this, name, args));
        return;
    }
    
    log("Spell %s not available.", name.c_str());
}

void Spellcaster::castByName(string name, const ValueMap& args)
{
    cast(name, args);
}

void Spellcaster::stop()
{
    if(crntSpell.get())
        crntSpell->end();
    crntSpell.reset();
}

void Spellcaster::update()
{
    if(crntSpell.get()){
        crntSpell->update();
    }
}

Spellcaster::~Spellcaster()
{
    if(crntSpell.get()){
        crntSpell->end();
    }
}

void DialogEntity::interact()
{
    GScene::crntScene->createDialog(getDialog(), false);
}

void Enemy::runDamageFlicker()
{
	if (sprite)
		sprite->runAction(flickerAction(0.3f, 1.2f, 81.0f));
//		sprite->runAction(colorFlickerAction(0.3f, 4, Color3B(255, 0, 0)));
}

void HitPointsEnemy::hit(int damage)
{
	runDamageFlicker();
    hp -= damage;
}

void HitPointsEnemy::update()
{
    if(hp == 0){
        GScene::getSpace()->removeObject(this);
    }
}

void TouchDamageEnemy::onTouchPlayer(Player* player){
	hitTarget = player;
}

void TouchDamageEnemy::endTouchPlayer()
{
	hitTarget = nullptr;
}

//hit will be registered every frame, in case contact is maintained for longer than the hit protection time.
void TouchDamageEnemy::update()
{
	if(hitTarget)
		hitTarget->hit();
}

void PlayerBulletDamage::onPlayerBulletHit(Bullet* bullet)
{
    hit(1);
}

void FrictionObject::update()
{
    SpaceVect vel = getVel();
    float force = getMass() * App::Gaccel * uk();
    
    //if acceleraion, dv/dt, or change in velocity over one frame is greater
    //than current velocity, apply stop instead
    if(App::Gaccel * uk() * App::secondsPerFrame < vel.length())
        applyForceForSingleFrame(vel * -force);
    else
        setVel(SpaceVect(0.0,0.0));
}
