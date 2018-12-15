//
//  GAnimation.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "GAnimation.hpp"
#include "macros.h"

shared_ptr<AnimationSpriteSequence> AnimationSpriteSequence::loadFromRasterImage(const string& path, int cols, int rows)
{
    Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(path);
    
    if(!texture){
        log("AnimationSpriteSequence: texture %s not loaded!", path.c_str());
        return shared_ptr<AnimationSpriteSequence>(nullptr);
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
    
    return make_shared<AnimationSpriteSequence>(frames);
}

shared_ptr<AnimationSpriteSequence> AnimationSpriteSequence::loadFromImageSequence(const string& name, int length)
{
    Vector<SpriteFrame*> frames;
    
    for(int i=1; i<= length; ++i)
    {
        string path = "sprites/" + name + boost::lexical_cast<string>(i) + ".png";
        
        if(!FileUtils::getInstance()->isFileExist(path)){
            log("loadFromImageSequence: %s not found", path.c_str());
            return nullptr;
        }
        
        frames.pushBack(Sprite::create(path)->getSpriteFrame());
    }
    
    return make_shared<AnimationSpriteSequence>(frames);
}

shared_ptr<AnimationSpriteSequence> AnimationSpriteSequence::loadAgentAnimation(const string& name)
{
	Vector<SpriteFrame*> frames;

	frames.pushBack(Sprite::create(name + "up-1.png")->getSpriteFrame());
	frames.pushBack(Sprite::create(name + "up-2.png")->getSpriteFrame());
	frames.pushBack(Sprite::create(name + "up-3.png")->getSpriteFrame());
	frames.pushBack(Sprite::create(name + "down-1.png")->getSpriteFrame());

	frames.pushBack(Sprite::create()->getSpriteFrame());
	frames.pushBack(Sprite::create()->getSpriteFrame());
	frames.pushBack(Sprite::create()->getSpriteFrame());
	frames.pushBack(Sprite::create(name + "down-2.png")->getSpriteFrame());

	frames.pushBack(Sprite::create(name + "right-1.png")->getSpriteFrame());
	frames.pushBack(Sprite::create(name + "right-2.png")->getSpriteFrame());
	frames.pushBack(Sprite::create(name + "right-3.png")->getSpriteFrame());
	frames.pushBack(Sprite::create(name + "down-3.png")->getSpriteFrame());

	frames.pushBack(Sprite::create()->getSpriteFrame());
	frames.pushBack(Sprite::create()->getSpriteFrame());
	frames.pushBack(Sprite::create()->getSpriteFrame());
	frames.pushBack(Sprite::create()->getSpriteFrame());

	return make_shared<AnimationSpriteSequence>(frames);
}



void TimedLoopAnimation::loadAnimation(const string& name, int length, SpaceFloat animationInterval)
{
    setName("TimedLoopAnimation");
    sequence = AnimationSpriteSequence::loadFromImageSequence(name, length);
    frameInterval = animationInterval / length;
    
    sprite = Sprite::createWithSpriteFrame(sequence->frames.at(0));
    sprite->setName("TimedLoopAnimation sprite");
    addChild(sprite,1);
}

void TimedLoopAnimation::update()
{
    timeInFrame += App::secondsPerFrame;
    
    while(timeInFrame >= frameInterval){
        ++crntFrame;
        timeInFrame -= frameInterval;
        if(crntFrame >= sequence->frames.size()){
            crntFrame = 0;
        }
    }
    
    sprite->setSpriteFrame(sequence->frames.at(crntFrame));
}

void PatchConAnimation::loadAnimation(const string& path)
{
    if(sprite)
        sprite->removeFromParent();
    
	if (path.back() == '/') {
		sequence = AnimationSpriteSequence::loadAgentAnimation(path);
		sprite = Sprite::createWithSpriteFrame(sequence->frames.at(0));
	}
	else {
		sequence = AnimationSpriteSequence::loadFromRasterImage(path, 4, 4);
		sprite = Sprite::createWithSpriteFrame(sequence->frames.at(0));
	}

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
    //flip or unflip sprite as needed
    sprite->setFlippedX(dir == Direction::left);
    direction = dir;
    
    //update sprite frame
    setFrame(crntFrame);
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
    int index = 0;
    //set crnt frame, and set sprite's frame to match
    switch(direction)
    {
    case Direction::up: index = animFrame; break;
    case Direction::left: case Direction::right: index = 8+ animFrame; break;
    case Direction::down: index = animFrame*4+3; break;
	case Direction::none: break;
    }
    sprite->setSpriteFrame(sequence->frames.at(index));
}

