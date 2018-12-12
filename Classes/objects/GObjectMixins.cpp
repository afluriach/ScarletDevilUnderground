//
//  GObjectMixins.cpp
//  Koumachika
//
//  Created by Toni on 4/15/18.
//
//

#include "Prefix.h"

#include "App.h"
#include "GAnimation.hpp"
#include "GObjectMixins.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "Player.hpp"
#include "scenes.h"
#include "Spell.hpp"
#include "util.h"
#include "value_map.hpp"

//LOGIC MIXINS

void DialogEntity::interact()
{
    GScene::crntScene->createDialog(getDialog(), false);
}

//END LOGIC

//PHYSICS MIXINS

void RectangleBody::initializeBody(GSpace& space)
{
    body = move(space.createRectangleBody(
        initialCenter,
        getDimensions(),
        getMass(),
        getType(),
        getLayers(),
        getSensor(),
        this
    ));
}

float RectangleBody::getMomentOfInertia() const{
    return rectangleMomentOfInertia(getMass(), getDimensions());
}

SpaceVect RectangleMapBody::getDimensionsFromMap(const ValueMap& arg)
{
    return SpaceVect(getFloat(arg, "dim_x"), getFloat(arg, "dim_y"));
}

void CircleBody::initializeBody(GSpace& space)
{
    body = move(space.createCircleBody(
        initialCenter,
        getRadius(),
        getMass(),
        getType(),
        getLayers(),
        getSensor(),
        this
    ));
}

float CircleBody::getMomentOfInertia() const{
    return circleMomentOfInertia(getMass(), getRadius());
}

void FrictionObject::update()
{
    //linear
    SpaceVect vel = getVel();
    float force = getMass() * App::Gaccel * uk();
    
    //if acceleraion, dv/dt, or change in velocity over one frame is greater
    //than current velocity, apply stop instead
    if(App::Gaccel * uk() * App::secondsPerFrame < vel.length())
        applyForceForSingleFrame(vel * -force);
    else
        setVel(SpaceVect::zero);
    
    //rotational
    float angularVel = getAngularVel();
    float angularImpulse = getMomentOfInertia() * App::Gaccel * uk() * App::secondsPerFrame;
    
    if(angularImpulse < angularVel)
        setAngularVel(angularVel - angularImpulse);
    else
        setAngularVel(0);
}

void DirectionalLaunch::init()
{
	setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));
}

//END PHYSICS

//GRAPHICS MIXINS

void ImageSprite::initializeGraphics(Layer* layer)
{
    loadImageSprite(imageSpritePath(), sceneLayer(), layer);
}

void ImageSprite::update()
{
    if(sprite != nullptr){
        sprite->setRotation(90-toDegrees(getAngle()));
    }
}

void ImageSprite::setSpriteShader(const string& shaderName)
{
    if(sprite != nullptr){
        Sprite* s = dynamic_cast<Sprite*>(sprite);
        if(s){
            s->setShader(shaderName);
        }
    }
}

void LoopAnimationSprite::initializeGraphics(Layer* layer)
{
    anim = Node::ccCreate<TimedLoopAnimation>();
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
    return 1.0f * App::pixelsPerTile / pixelWidth();
}

void PatchConSprite::init()
{
    setDirection(startingDirection);
}

void PatchConSprite::initializeGraphics(Layer* layer)
{
    animSprite = Node::ccCreate<PatchConAnimation>();
    animSprite->loadAnimation(imageSpritePath());
    layer->positionAndAddNode(animSprite, sceneLayerAsInt(), getInitialCenterPix(), zoom());
    sprite = animSprite;
}

void PatchConSprite::setSprite(const string& name)
{
    animSprite->loadAnimation("sprites/"+name+".png");
}

void PatchConSprite::setSpriteShader(const string& shaderName)
{
    if(sprite != nullptr){
        PatchConAnimation* p = dynamic_cast<PatchConAnimation*>(sprite);
        if(p){
            p->setSpriteShader(shaderName);
        }
    }
}

void PatchConSprite::update()
{
    SpaceVect dist = body->getVel()*App::secondsPerFrame;
    
    animSprite->accumulate(dist.length());
}

void PatchConSprite::setAngle(float a)
{
    GObject::setAngle(a);
    
	animSprite->setDirection(angleToDirection(a));
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
		hitTarget->hit(1, nullptr);
}

//END ENEMY
