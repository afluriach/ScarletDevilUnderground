//
//  GObjectMixins.cpp
//  Koumachika
//
//  Created by Toni on 4/15/18.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "app_constants.hpp"
#include "audio_context.hpp"
#include "FloorSegment.hpp"
#include "GAnimation.hpp"
#include "GObjectMixins.hpp"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "object_ref.hpp"
#include "physics_context.hpp"
#include "value_map.hpp"
#include "Wall.hpp"

//LOGIC MIXINS

void DialogEntity::interact(Player* p)
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

RectangleBody::RectangleBody(const ValueMap& arg) : dim(getObjectDimensions(arg)) {}

void RectangleBody::initializeBody(GSpace& space)
{
	tie(bodyShape, body) = space.physicsContext->createRectangleBody(
        initialCenter,
        dim,
        getMass(),
        getType(),
        getLayers(),
        getSensor(),
        this
    );
}

void CircleBody::initializeBody(GSpace& space)
{
    tie(bodyShape, body) = space.physicsContext->createCircleBody(
        initialCenter,
        getRadius(),
        getMass(),
        getType(),
        getLayers(),
        getSensor(),
        this
    );
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
	loadImageSprite();
}

void LoopAnimationSprite::initializeGraphics()
{
	sprite_properties sprite = app::getSprite(getSprite());
	float zoom = getSpriteZoom(sprite, getRadius());

	animID = space->createSprite(
		&graphics_context::createLoopAnimation,
		sprite.filename,
		sprite.size.first,
		animationDuration(),
		sceneLayer(),
		getInitialCenterPix(),
		zoom
	);

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

void PatchConSprite::init()
{
}

void PatchConSprite::initializeGraphics()
{
	spriteID = space->createSprite(
		&graphics_context::createAgentSprite,
		getSprite(),
		getRadius(),
		sceneLayer(),
		getInitialCenterPix()
	);

	setDirection(startingDirection);
}

void PatchConSprite::setSprite(const string& name)
{
	if (spriteID != 0) {
		space->graphicsNodeAction(
			&PatchConAnimation::loadAnimation,
			spriteID,
			name
		);
	}
}

void PatchConSprite::_update()
{
    SpaceVect dist = getVel()*app::params.secondsPerFrame;
    
    bool advance = accumulate(dist.length());

	if (advance && crntFloorCenterContact.isValid() ) {
		string sfxRes = crntFloorCenterContact.get()->getFootstepSfx();
		if (!sfxRes.empty()) {
			space->audioContext->playSoundSpatial(sfxRes, toVec3(getPos(), -1.0), toVec3(getVel()), 0.5f);
		}
	}
}

void PatchConSprite::setAngle(SpaceFloat a)
{
    GObject::setAngle(a);

	space->graphicsNodeAction(&PatchConAnimation::setDirection, spriteID, angleToDirection(a));
}

void PatchConSprite::setDirection(Direction d)
{
    GObject::setDirection(d);
    if(d == Direction::none) return;

	space->graphicsNodeAction(&PatchConAnimation::setDirection, spriteID, d);
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
			space->graphicsNodeAction(&PatchConAnimation::setFrame, spriteID, 1);
			crntFrame = 1;
			advance = true;
			accumulator -= stepSize;
			nextStepIsLeft = false;
		}
		break;
	case 2:
		if (accumulator >= stepSize)
		{
			space->graphicsNodeAction(&PatchConAnimation::setFrame, spriteID, 1);
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
			space->graphicsNodeAction(&PatchConAnimation::setFrame, spriteID, crntFrame);
			accumulator -= midstepSize;
		}
		break;
	}

	return advance;
}

void PatchConSprite::reset()
{
	space->graphicsNodeAction(&PatchConAnimation::setFrame, spriteID, 1);
	accumulator = 0.0;

	nextStepIsLeft = firstStepIsLeft;
	//Toggle which foot will be used to take the first step next time.
	firstStepIsLeft = !firstStepIsLeft;
}

void ImageSprite::loadImageSprite()
{
	sprite_properties _sprite = app::getSprite(getSprite());
	float zoom = getSpriteZoom(_sprite, getRadius());
	string resPath = "sprites/" + _sprite.filename + ".png";

	spriteID = space->createSprite(
		&graphics_context::createSprite,
		resPath,
		sceneLayer(),
		getInitialCenterPix(),
		zoom
	);
	space->graphicsNodeAction(&Node::setRotation, spriteID, toCocosAngle(prevAngle));

	if (spriteID != 0 && _sprite.color != Color3B::BLACK && _sprite.color != Color3B::WHITE) {
		space->graphicsNodeAction(&Node::setColor, spriteID, _sprite.color);
	}

}

LightObject::LightObject() :
	RegisterInit<LightObject>(this)
{}

void LightObject::init()
{
	auto source = getLightSource();
	if (source) {
		lightID = space->addLightSource(source);
		space->addGraphicsAction(&graphics_context::setLightSourcePosition, lightID, prevPos);
		space->addGraphicsAction(&graphics_context::setLightSourceAngle, lightID, prevAngle);
	}
}

//END GRAPHICS

//BEGIN AUDIO

void AudioSourceObject::_update()
{
	auto it = sources.begin();
	while (it != sources.end())
	{
		ALuint sourceID = *it;
		if (!space->audioContext->setSoundSourcePos(sourceID, getPos(), getVel(), getAngle()))
			it = sources.erase(it);
		else
			++it;
	}
}

ALuint AudioSourceObject::playSoundSpatial(const string& path, float volume, bool loop )
{
	ALuint soundSource = space->audioContext->playSoundSpatial(path, toVec3(getPos()), toVec3(getVel()), volume, loop);

	if (soundSource != 0) sources.push_back(soundSource);
	return soundSource;
}

void AudioSourceObject::stopSound(ALuint sourceID)
{
	space->audioContext->endSound(sourceID);
	sources.remove(sourceID);
}

bool AudioSourceObject::isSourceActive(ALuint source)
{
	return space->audioContext->isSoundSourceActive(source);
}

//END AUDIO
