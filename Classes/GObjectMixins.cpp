//
//  GObjectMixins.cpp
//  Koumachika
//
//  Created by Toni on 4/15/18.
//
//

#include "App.h"
#include "GAnimation.hpp"
#include "GObjectMixins.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "PLayer.hpp"
#include "scenes.h"
#include "Spell.hpp"
#include "util.h"

//LOGIC MIXINS

void DialogEntity::interact()
{
    GScene::crntScene->createDialog(getDialog(), false);
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
        if(crntSpell->isActive())
            crntSpell->update();
        else
            crntSpell.reset();
    }
}

Spellcaster::~Spellcaster()
{
    if(crntSpell.get()){
        crntSpell->end();
    }
}

//END LOGIC

//PHYSICS MIXINS

void RectangleBody::initializeBody(GSpace& space)
{
    body = space.createRectangleBody(
        initialCenter,
        getDimensions(),
        getMass(),
        getType(),
        getLayers(),
        getSensor(),
        this
    );
}

SpaceVect RectangleMapBody::getDimensionsFromMap(const ValueMap& arg)
{
    return SpaceVect(getFloat(arg, "dim_x"), getFloat(arg, "dim_y"));
}

void CircleBody::initializeBody(GSpace& space)
{
    body = space.createCircleBody(
        initialCenter,
        getRadius(),
        getMass(),
        getType(),
        getLayers(),
        getSensor(),
        this
    );
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
        setVel(SpaceVect::zero);
}

//END PHYSICS

//GRAPHICS MIXINS

void SpriteObject::update()
{
    if(sprite != nullptr){
        sprite->setPosition(toCocos(body->getPos())*App::pixelsPerTile);
    }
}

void ImageSprite::initializeGraphics(Layer* layer)
{
    loadImageSprite(imageSpritePath(), sceneLayer(), layer);
}

void ImageSprite::update()
{
    if(sprite != nullptr){
        sprite->setRotation(-toDegrees(getAngle()));
    }
}

void LoopAnimationSprite::initializeGraphics(Layer* layer)
{
    anim = TimedLoopAnimation::create();
    anim->loadAnimation(animationName(), animationSize(), animationDuration());
    
    layer->positionAndAddNode(anim, sceneLayerAsInt(), getInitialCenterPix(), zoom());
    sprite = anim;
}

void LoopAnimationSprite::update()
{
    anim->update();
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

float PatchConSprite::zoom() const
{
    return App::pixelsPerTile / pixelWidth;
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

void PatchConSprite::setAngle(float a)
{
    GObject::setAngle(a);
    
    setDirection(angleToDirection(a));
}

void PatchConSprite::setDirection(Direction d)
{
    GObject::setDirection(d);
    if(d == Direction::none) return;

    animSprite->setDirection(d);
}

Direction PatchConSprite::getDirection() const
{
    return animSprite->getDirection();
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

//ENEMY MIXINS

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

//END ENEMY