//
//  GObjectMixins.cpp
//  Koumachika
//
//  Created by Toni on 4/15/18.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "FloorSegment.hpp"
#include "GAnimation.hpp"
#include "GObjectMixins.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "object_ref.hpp"
#include "Player.hpp"
#include "SpaceLayer.h"
#include "scenes.h"
#include "Spell.hpp"
#include "util.h"
#include "value_map.hpp"
#include "Wall.hpp"

//LOGIC MIXINS

void DialogEntity::interact()
{
	space->getScene()->createDialog(getDialog(), false);
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

SpaceFloat RectangleBody::getMomentOfInertia() const{
    return rectangleMomentOfInertia(getMass(), getDimensions());
}

CCRect RectangleBody::getBoundingBox()
{
	SpaceVect pos = getPos();
	SpaceVect dim = getDimensions();
	CCRect result(pos.x - dim.x/2, pos.y - dim.y/2, dim.x, dim.y);

	return result;
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

SpaceFloat CircleBody::getMomentOfInertia() const{
    return circleMomentOfInertia(getMass(), getRadius());
}

CCRect CircleBody::getBoundingBox()
{
	SpaceVect pos = getPos();
	SpaceFloat radius = getRadius();
	SpaceVect dim(radius*2, radius*2);
	CCRect result(pos.x - dim.x / 2, pos.y - dim.y / 2, dim.x, dim.y);

	return result;
}

SpaceVect CircleBody::getDimensions() const
{
	return SpaceVect(getRadius()*2, getRadius()*2);
}

void FrictionObject::update()
{
	SpaceFloat frictionCoeff = crntFloorCenterContact.isValid() ? crntFloorCenterContact.get()->getFrictionCoeff() : 1.0;

    //linear
    SpaceVect vel = getVel();
	SpaceFloat force = getMass() * App::Gaccel * uk() * frictionCoeff;
    
    //if acceleraion, dv/dt, or change in velocity over one frame is greater
    //than current velocity, apply stop instead
    if(App::Gaccel * uk() * frictionCoeff * App::secondsPerFrame < vel.length())
        applyForceForSingleFrame(vel * -force);
    else
        setVel(SpaceVect::zero);
    
    //rotational
	SpaceFloat angularVel = getAngularVel();
	SpaceFloat angularImpulse = getMomentOfInertia() * App::Gaccel * uk() * frictionCoeff * App::secondsPerFrame;
    
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

void ImageSprite::initializeGraphics(SpaceLayer* spaceLayer)
{
    loadImageSprite(imageSpritePath(), sceneLayer(), spaceLayer->getLayer(sceneLayer()));
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

void LoopAnimationSprite::initializeGraphics(SpaceLayer* layer)
{
    anim = Node::ccCreate<TimedLoopAnimation>();
    anim->loadAnimation(animationName(), animationSize(), animationDuration());
    
    layer->getLayer(sceneLayer())->positionAndAddNode(anim, 1, getInitialCenterPix(), zoom());
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

void PatchConSprite::initializeGraphics(SpaceLayer* layer)
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

void PatchConSprite::setAngle(SpaceFloat a)
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

//END GRAPHICS

//TYPE MIXINS

void Bullet::onWallCollide(Wall* wall)
{
	space->removeObject(this);
}

void Bullet::onEnvironmentCollide(GObject* obj)
{
	space->removeObject(this);
}

void Bullet::onAgentCollide(Agent* agent)
{
	agent->hit(getAttributeEffect(), getMagicEffect(agent));
	space->removeObject(this);
}

void PlayerShield::onWallCollide(Wall* wall)
{
	//NO-OP
}

void PlayerShield::onEnvironmentCollide(GObject* obj)
{
	//NO-OP
}

void PlayerShield::onAgentCollide(Agent* agent)
{
	agent->hit(getAttributeEffect(), getMagicEffect(agent));
}

EnemyBullet::EnemyBullet(SpaceFloat grazeRadius, int grazeBonus) :
grazeRadius(grazeRadius),
grazeBonus(grazeBonus)
{}

void EnemyBullet::onGrazeTouch(object_ref<Player> p)
{
	grazeTarget = p;
}

void EnemyBullet::invalidateGraze()
{
	grazeValid = false;
}

void EnemyBullet::onGrazeCleared(object_ref<Player> p)
{
	if(grazeTarget == p && p.isValid() && grazeValid)
	{
		p.get()->applyGraze(grazeBonus);
	}
}

void EnemyBullet::initializeRadar(GSpace& space)
{
	if (grazeRadius <= 0.0)
		return;

	radar = space.createCircleBody(
		initialCenter,
		grazeRadius,
		0.1,
		GType::enemyBulletRadar,
		PhysicsLayers::all,
		true,
		this
	);
}

//END TYPE
