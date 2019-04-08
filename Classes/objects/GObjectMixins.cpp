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
#include "GObjectMixins.hpp"
#include "GScene.hpp"
#include "GSpace.hpp"
#include "object_ref.hpp"
#include "util.h"
#include "value_map.hpp"
#include "Wall.hpp"

//LOGIC MIXINS

void DialogEntity::interact()
{
	space->createDialog(
		getDialog(),
		false,
		[this]()->void {onDialogEnd(); }
	);
}

DialogImpl::DialogImpl(const ValueMap& args)
{
	dialogName = getStringOrDefault(args, "dialog", "");
}

//END LOGIC

//PHYSICS MIXINS

void RectangleBody::initializeBody(GSpace& space)
{
	tie(bodyShape, body) = space.createRectangleBody(
        initialCenter,
        getDimensions(),
        getMass(),
        getType(),
        getLayers(),
        getSensor(),
        this
    );
}

SpaceFloat RectangleBody::getMomentOfInertia() const{
    return rectangleMomentOfInertia(getMass(), getDimensions());
}

SpaceRect RectangleBody::getBoundingBox()
{
	return SpaceRect(getPos(), getDimensions());
}

SpaceVect RectangleBody::getDimensionsFromMap(const ValueMap& arg)
{
    return SpaceVect(getFloat(arg, "dim_x"), getFloat(arg, "dim_y"));
}

void CircleBody::initializeBody(GSpace& space)
{
    tie(bodyShape, body) = space.createCircleBody(
        initialCenter,
        getRadius(),
        getMass(),
        getType(),
        getLayers(),
        getSensor(),
        this
    );
}

SpaceFloat CircleBody::getMomentOfInertia() const{
    return circleMomentOfInertia(getMass(), getRadius());
}

SpaceRect CircleBody::getBoundingBox()
{
	SpaceFloat radius = getRadius();
	return SpaceRect(getPos(), SpaceVect(radius*2.0, radius*2.0));
}

SpaceVect CircleBody::getDimensions() const
{
	return SpaceVect(getRadius()*2, getRadius()*2);
}

ParametricMotion::ParametricMotion(parametric_space_function f, SpaceFloat start, SpaceFloat scale) :
	scale(scale),
	f(f),
	t(start),
	RegisterInit<ParametricMotion>(this)
{}

void ParametricMotion::init()
{
	origin = getPos();
}

void ParametricMotion::_update()
{
	setPos(f(t) + origin);
	timerIncrement(t, scale);
}

//END PHYSICS

//GRAPHICS MIXINS

void ImageSprite::initializeGraphics()
{
    loadImageSprite(imageSpritePath(), sceneLayer());
}

void LoopAnimationSprite::initializeGraphics()
{
	animID = space->createLoopAnimation(animationName(), animationSize(), animationDuration(), sceneLayer(), getInitialCenterPix(), zoom());
	spriteID = animID;
}

PatchConSprite::PatchConSprite(const ValueMap& args) :
	PatchConSprite(getDirectionOrDefault(args, Direction::up))
{
}

PatchConSprite::PatchConSprite(Direction startingDirection) :
	RegisterInit<PatchConSprite>(this),
	startingDirection(startingDirection)
{
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
	spriteID = space->createAgentSprite(imageSpritePath(), isAgentAnimation(), sceneLayer(), getInitialCenterPix(), zoom());
}

void PatchConSprite::setSprite(const string& name)
{
	setSprite(name, isAgentAnimation());
}

void PatchConSprite::setSprite(const string& name, bool agentAnimation)
{
	if (spriteID != 0) {
		space->loadAgentAnimation(spriteID, "sprites/" + name + ".png", agentAnimation);
	}
}

void PatchConSprite::_update()
{
    SpaceVect dist = getVel()*App::secondsPerFrame;
    
    bool advance = accumulate(dist.length());

	if (advance && crntFloorCenterContact.isValid() ) {
		string sfxRes = crntFloorCenterContact.get()->getFootstepSfx();
		if (!sfxRes.empty()) {
			App::playSoundSpatial(sfxRes, toVec3(getPos(), -1.0), toVec3(getVel()), 0.5f);
		}
	}
}

void PatchConSprite::setAngle(SpaceFloat a)
{
    GObject::setAngle(a);

	space->setAgentAnimationDirection(spriteID, angleToDirection(a));
}

void PatchConSprite::setDirection(Direction d)
{
    GObject::setDirection(d);
    if(d == Direction::none) return;

	space->setAgentAnimationDirection(spriteID, d);
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
			space->setAgentAnimationFrame(spriteID, 1);
			crntFrame = 1;
			advance = true;
			accumulator -= stepSize;
			nextStepIsLeft = false;
		}
		break;
	case 2:
		if (accumulator >= stepSize)
		{
			space->setAgentAnimationFrame(spriteID, 1);
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
			space->setAgentAnimationFrame(spriteID, crntFrame);
			accumulator -= midstepSize;
		}
		break;
	}

	return advance;
}

void PatchConSprite::reset()
{
	space->setAgentAnimationFrame(spriteID, 1);
	accumulator = 0.0;

	nextStepIsLeft = firstStepIsLeft;
	//Toggle which foot will be used to take the first step next time.
	firstStepIsLeft = !firstStepIsLeft;
}

void ImageSprite::loadImageSprite(const string& resPath, GraphicsLayer sceneLayer)
{
	spriteID = space->createSprite(resPath, sceneLayer, getInitialCenterPix(), zoom());
	space->setSpriteAngle(spriteID, toCocosAngle(prevAngle));
}

RadialLightObject::RadialLightObject() :
	RegisterInit<RadialLightObject>(this)
{}

void RadialLightObject::init()
{
	lightID = space->addLightSource(getLightSource());
}

SpriteLightObject::SpriteLightObject() :
	RegisterInit<SpriteLightObject>(this)
{}

void SpriteLightObject::init()
{
	lightID = space->addLightSource(getLightSource());

	space->setLightSourcePosition(lightID, prevPos);
	space->setLightSourceAngle(lightID, prevAngle);
}


//END GRAPHICS

//BEGIN AUDIO

void AudioSourceObject::_update()
{
	auto it = sources.begin();
	while (it != sources.end())
	{
		ALuint sourceID = *it;
		if (!App::setSoundSourcePos(sourceID, getPos(), getVel(), getAngle()))
			it = sources.erase(it);
		else
			++it;
	}
}

ALuint AudioSourceObject::playSoundSpatial(const string& path, float volume, bool loop )
{
	ALuint soundSource = App::playSoundSpatial(path, toVec3(getPos()), toVec3(getVel()), volume, loop);

	if (soundSource != 0) sources.push_back(soundSource);
	return soundSource;
}

void AudioSourceObject::stopSound(ALuint sourceID)
{
	App::endSound(sourceID);
	sources.remove(sourceID);
}

bool AudioSourceObject::isSourceActive(ALuint source)
{
	return App::isSoundSourceActive(source);
}

//END AUDIO
