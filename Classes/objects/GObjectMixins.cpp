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

void ImageSprite::initializeGraphics()
{
    loadImageSprite(imageSpritePath(), sceneLayer());
}

void ImageSprite::update()
{
    if(spriteID != 0){
		space->getScene()->setSpriteAngle(spriteID, 90 - toDegrees(getAngle()));
    }
}

void LoopAnimationSprite::initializeGraphics()
{
	animID = space->getScene()->createLoopAnimation(animationName(), animationSize(), animationDuration(), sceneLayer(), getInitialCenterPix(), zoom());
	spriteID = animID;
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

void PatchConSprite::initializeGraphics()
{
	spriteID = space->getScene()->createAgentSprite(imageSpritePath(), isAgentAnimation(), sceneLayer(), getInitialCenterPix(), zoom());
}

void PatchConSprite::setSprite(const string& name)
{
	if (spriteID != 0) {
		space->getScene()->loadAgentAnimation(spriteID, "sprites/" + name + ".png", isAgentAnimation());
	}
}

void PatchConSprite::update()
{
    SpaceVect dist = body->getVel()*App::secondsPerFrame;
    
    bool advance = accumulate(dist.length());

	if (advance && crntFloorCenterContact.isValid() ) {
		string sfxRes = crntFloorCenterContact.get()->getFootstepSfx();
		if (!sfxRes.empty()) {
			if (!dynamic_cast<Player*>(this)) {
				App::playSoundSpatial(sfxRes, getPos(), getVel());
			}
		}
	}
}

void PatchConSprite::setAngle(SpaceFloat a)
{
    GObject::setAngle(a);

	space->getScene()->setAgentAnimationDirection(spriteID, angleToDirection(a));
}

void PatchConSprite::setDirection(Direction d)
{
    GObject::setDirection(d);
    if(d == Direction::none) return;

	space->getScene()->setAgentAnimationDirection(spriteID, d);
}

bool PatchConSprite::accumulate(SpaceFloat dx)
{
	accumulator += dx;
	return checkAdvanceAnimation();
}

bool PatchConSprite::checkAdvanceAnimation()
{
	bool advance = false;

	//TODO cases are symmetrical, should be able to optimize
	switch (crntFrame)
	{
	case 0:
		if (accumulator >= stepSize)
		{
			space->getScene()->setAgentAnimationFrame(spriteID, 1);
			crntFrame = 1;
			advance = true;
			accumulator -= stepSize;
			nextStepIsLeft = false;
		}
		break;
	case 2:
		if (accumulator >= stepSize)
		{
			space->getScene()->setAgentAnimationFrame(spriteID, 1);
			crntFrame = 1;
			advance = true;
			accumulator -= stepSize;
			nextStepIsLeft = true;
		}
		break;
	case 1:
		if (accumulator >= midstepSize)
		{
			crntFrame = (nextStepIsLeft ? 0 : 2);
			space->getScene()->setAgentAnimationFrame(spriteID, crntFrame);
			accumulator -= midstepSize;
		}
		break;
	}

	return advance;
}

void PatchConSprite::reset()
{
	space->getScene()->setAgentAnimationFrame(spriteID, 1);
	accumulator = 0.0;

	nextStepIsLeft = firstStepIsLeft;
	//Toggle which foot will be used to take the first step next time.
	firstStepIsLeft = !firstStepIsLeft;
}

void ImageSprite::loadImageSprite(const string& resPath, GraphicsLayer sceneLayer)
{
	spriteID = space->getScene()->createSprite(resPath, sceneLayer, getInitialCenterPix(), zoom());
}

//END GRAPHICS
