//
//  GAnimation.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "enum.h"
#include "GAnimation.hpp"
#include "macros.h"

AnimationSpriteSequence AnimationSpriteSequence::loadFromRasterImage(const string& path, int cols, int rows)
{
    Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(path);
    
    if(!texture){
        log("AnimationSpriteSequence: texture %s not loaded!", path.c_str());
        return AnimationSpriteSequence();
    }
    
    Vector<SpriteFrame*> frames;
    
    cocos2d::CCSize size = texture->getContentSize();
    IntVec2 intSize = toIntVector(size);
    
    if(intSize.first % cols != 0 || intSize.second % rows != 0){
        log(
            "AnimationSpriteSequence: texture %s is not evenly divisible! %d,%d px at %d by %d",
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

AnimationSpriteSequence AnimationSpriteSequence::loadFromImageSequence(const string& name, int length)
{
    Vector<SpriteFrame*> frames;
    
    for(int i=1; i<= length; ++i)
    {
        string path = "sprites/" + name + boost::lexical_cast<string>(i) + ".png";
        
        if(!FileUtils::getInstance()->isFileExist(path)){
            log("loadFromImageSequence: %s not found", path.c_str());
            return AnimationSpriteSequence();
        }
        
        frames.pushBack(Sprite::create(path)->getSpriteFrame());
    }
    
    return AnimationSpriteSequence(frames);
}

SpriteFrame* loadSpriteFrame(const string& path)
{
	Texture2D* t = Director::getInstance()->getTextureCache()->addImage(path);
	return SpriteFrame::createWithTexture(t, CCRect(0, 0, t->getPixelsWide(), t->getPixelsHigh()));
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
    sequence = AnimationSpriteSequence::loadFromImageSequence(name, length);
    frameInterval = animationInterval / length;
    
    sprite = Sprite::createWithSpriteFrame(sequence.frames.at(0));
    sprite->setName("TimedLoopAnimation sprite");
    addChild(sprite,1);
}

void TimedLoopAnimation::update()
{
    timeInFrame += App::secondsPerFrame;
    
    while(timeInFrame >= frameInterval){
        ++crntFrame;
        timeInFrame -= frameInterval;
        if(crntFrame >= sequence.frames.size()){
            crntFrame = 0;
        }
    }
    
    sprite->setSpriteFrame(sequence.frames.at(crntFrame));
}

void PatchConAnimation::setSpriteShader(const string& shader) {
	sprite->setShader(shader);
}


void PatchConAnimation::loadAnimation(const string& path, bool agentAnimation)
{
	if(sprite)
        sprite->removeFromParent();

	if (agentAnimation) {
		walkAnimations = AnimationSpriteSequence::loadAgentAnimation(path);
	}
	else {
		walkAnimations = AnimationSpriteSequence::loadPatchconSpriteSheet(path);
		useFlipX = true;
	}

	sprite = Sprite::create();
    addChild(sprite,1);
    sprite->useAntiAliasTexture(false);
    
    this->setCascadeOpacityEnabled(true);
}


void PatchConAnimation::accumulate(SpaceFloat dx)
{
    distanceAccumulated += dx;
    checkAdvanceAnimation();
}

//Reset to standing. Implicitly removes run effect.
void PatchConAnimation::reset()
{
    setFrame(1);
    distanceAccumulated = 0;
    
    nextStepIsLeft = firstStepIsLeft;
    //Toggle which foot will be used to take the first step next time.
    firstStepIsLeft = !firstStepIsLeft;
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


void PatchConAnimation::checkAdvanceAnimation()
{
    //TODO cases are symmetrical, should be able to optimize
    switch(crntFrame)
    {
    case 0:
        if(distanceAccumulated >= stepSize)
        {
            setFrame(1);
            distanceAccumulated -= stepSize;
            nextStepIsLeft = false;
        }
    break;
    case 2:
        if(distanceAccumulated >= stepSize)
        {
            setFrame(1);
            distanceAccumulated -= stepSize;
            nextStepIsLeft = true;
        }
    break;
    case 1:
        if(distanceAccumulated >= midstepSize)
        {
            setFrame(nextStepIsLeft ? 0 : 2);
            distanceAccumulated -= midstepSize;
        }
    break;
    }
}

void PatchConAnimation::setFrame(int animFrame)
{
    crntFrame = animFrame;

	if (direction != Direction::none){
		sprite->setSpriteFrame(walkAnimations.at(to_size_t(direction) - 1).frames.at(animFrame));
	}

	if (useFlipX) {
		sprite->setFlippedX(direction == Direction::left);
	}
}

