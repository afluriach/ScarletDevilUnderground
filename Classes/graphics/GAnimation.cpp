//
//  GAnimation.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "GAnimation.hpp"
#include "graphics_context.hpp"

AnimationSpriteSequence AnimationSpriteSequence::loadFromRasterImage(const string& path, int cols, int rows)
{
	Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(path);
	
	if(!texture){
		log1("AnimationSpriteSequence: texture %s not loaded!", path.c_str());
		return AnimationSpriteSequence();
	}
	
	Vector<SpriteFrame*> frames;
	
	cocos2d::CCSize size = texture->getContentSize();
	IntVec2 intSize = toIntVector(size);
	
	if(intSize.first % cols != 0 || intSize.second % rows != 0){
		log5(
			"texture %s is not evenly divisible! %d,%d px at %d by %d",
			path.c_str(),
			intSize.first,
			intSize.second,
			cols,
			rows
		);
	}
	
	int frameWidth = intSize.first / cols;
	int frameHeight = intSize.second / rows;
	
	for_irange(r,0,rows)
	{
		for_irange(c,0,cols)
		{
			cocos2d::CCRect rect(c*frameWidth,r*frameHeight,frameWidth,frameHeight);
			frames.pushBack(SpriteFrame::createWithTexture(texture, rect));
		}
	}
	
	return AnimationSpriteSequence(frames);
}

array<AnimationSpriteSequence, 4> AnimationSpriteSequence::loadPatchconSpriteSheet(const string& path)
{
	array<AnimationSpriteSequence, 4> result;
	AnimationSpriteSequence spriteSheet = loadFromRasterImage(path, 4, 4);

	result[0] = AnimationSpriteSequence(spriteSheet.frames.begin()+8, spriteSheet.frames.begin() + 11);
	result[1] = AnimationSpriteSequence(spriteSheet.frames.begin(), spriteSheet.frames.begin() + 3);

	result[2] = AnimationSpriteSequence();
	result[2].frames.pushBack(spriteSheet.frames.at(10));
	result[2].frames.pushBack(spriteSheet.frames.at(9));
	result[2].frames.pushBack(spriteSheet.frames.at(8));

	result[3] = AnimationSpriteSequence();
	result[3].frames.pushBack(spriteSheet.frames.at(3));
	result[3].frames.pushBack(spriteSheet.frames.at(7));
	result[3].frames.pushBack(spriteSheet.frames.at(11));

	return result;
}

array<AnimationSpriteSequence, 4> AnimationSpriteSequence::loadAgentAnimation(const string& path)
{
	array<AnimationSpriteSequence, 4> result;
	AnimationSpriteSequence spriteSheet = loadFromRasterImage(path, 3, 4);

	for_irange(row, 0, 4){
		result[row] = AnimationSpriteSequence();
		for_irange(col, 0, 3){
			result[row].frames.pushBack(spriteSheet.frames.at(row * 3 + col));
		}
	}

	return result;
}

AnimationSpriteSequence::AnimationSpriteSequence(Vector<SpriteFrame*> frames) : frames(frames) {}

AnimationSpriteSequence::AnimationSpriteSequence() {}

AnimationSpriteSequence::AnimationSpriteSequence(Vector<SpriteFrame*>::iterator begin, Vector<SpriteFrame*>::iterator end)
{
	for (auto it = begin; it != end; ++it) {
		frames.pushBack(*it);
	}
}

void TimedLoopAnimation::loadAnimation(const string& name, int length, SpaceFloat animationInterval)
{
	setName("TimedLoopAnimation");
	sequence = AnimationSpriteSequence::loadFromRasterImage("sprites/"+name+".png", length, 1);
	frameInterval = animationInterval / length;

	setSpriteFrame(sequence.frames.at(0));
}

bool TimedLoopAnimation::init()
{
	return Sprite::init();
}

void TimedLoopAnimation::update(float unused)
{
	timerIncrement(timeInFrame);
	
	while(timeInFrame >= frameInterval){
		++crntFrame;
		timeInFrame -= frameInterval;
		if(crntFrame >= sequence.frames.size()){
			crntFrame = 0;
		}
	}
	
	setSpriteFrame(sequence.frames.at(crntFrame));
}

bool PatchConAnimation::init()
{
	return Sprite::init();
}

void PatchConAnimation::loadAnimation(shared_ptr<sprite_properties> _sprite)
{
	sprite_properties _props = *_sprite.get();
	string path = "sprites/" + _props.filename + ".png";
	bool agentAnimation = _props.size == make_pair(3, 4);

	if (agentAnimation) {
		walkAnimations = AnimationSpriteSequence::loadAgentAnimation(path);
	}
	else if(_props.size == make_pair(4,4)){
		walkAnimations = AnimationSpriteSequence::loadPatchconSpriteSheet(path);
	}
	else {
		log1("Invalid agent sprite %s.", _sprite);
		return;
	}

	useFlipX = !agentAnimation;

	useAntiAliasTexture(false);
	setCascadeOpacityEnabled(true);
	setFrame(crntFrame);
}

void PatchConAnimation::setDirection(Direction dir)
{
	direction = dir;
	
	//update sprite frame
	setFrame(crntFrame);
}

Direction PatchConAnimation::getDirection()const {
	return direction;
}

void PatchConAnimation::setFrame(int animFrame)
{
	crntFrame = animFrame;

	if (direction != Direction::none){
		setSpriteFrame(walkAnimations.at(to_size_t(direction) - 1).frames.at(animFrame));
	}

	if (useFlipX) {
		setFlippedX(direction == Direction::left);
	}
}

node_context AgentAnimationContext::initializeGraphics(
	shared_ptr<sprite_properties> sprite,
	SpaceFloat radius,
	GraphicsLayer glayer,
	Vec2 centerPix
){
	spriteContext.createNode(
		&graphics_context::createAgentSprite,
		sprite,
		radius,
		glayer,
		centerPix
	);

	setDirection(startingDirection);
	return spriteContext;
}

void AgentAnimationContext::setSprite(shared_ptr<sprite_properties> sprite)
{
	if (spriteContext) {
		spriteContext.nodeMethod(
			&PatchConAnimation::loadAnimation,
			sprite
		);
	}
}

void AgentAnimationContext::setSpriteShader(string shader)
{
	if (spriteContext) {
		spriteContext.setShader(shader);
	}
}

bool AgentAnimationContext::accumulate(SpaceFloat dx)
{
	accumulator += dx;
	return checkAdvanceAnimation();
}

bool AgentAnimationContext::checkAdvanceAnimation()
{
	bool advance = false;

	//TODO cases are symmetrical, should be able to optimize
	switch (crntFrame)
	{
	case 0:
		if (accumulator >= stepSize)
		{
			spriteContext.nodeMethod(&PatchConAnimation::setFrame, 1);
			crntFrame = 1;
			advance = true;
			accumulator -= stepSize;
			nextStepIsLeft = false;
		}
		break;
	case 2:
		if (accumulator >= stepSize)
		{
			spriteContext.nodeMethod(&PatchConAnimation::setFrame, 1);
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
			spriteContext.nodeMethod(&PatchConAnimation::setFrame, crntFrame);
			accumulator -= midstepSize;
		}
		break;
	}

	return advance;
}

void AgentAnimationContext::reset()
{
	spriteContext.nodeMethod(&PatchConAnimation::setFrame, 1);
	accumulator = 0.0;

	nextStepIsLeft = firstStepIsLeft;
	//Toggle which foot will be used to take the first step next time.
	firstStepIsLeft = !firstStepIsLeft;
}

void AgentAnimationContext::setAngle(SpaceFloat a)
{
	spriteContext.nodeMethod(&PatchConAnimation::setDirection, angleToDirection(a));
}

void AgentAnimationContext::setDirection(Direction d)
{
	if (d != Direction::none) {
		spriteContext.nodeMethod(&PatchConAnimation::setDirection, d);
	}
}
