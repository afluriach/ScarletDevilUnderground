//
//  GAnimation.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

//Already in PCH
//#include "GAnimation.hpp"

shared_ptr<AnimationSpriteSequence> AnimationSpriteSequence::loadFromRasterImage(const string& path, int cols, int rows)
{
    Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(path);
    
    if(!texture){
        log("AnimationSpriteSequence: texture %s not loaded!", path.c_str());
        return shared_ptr<AnimationSpriteSequence>(nullptr);
    }
    
    Vector<SpriteFrame*> frames;
    
    cocos2d::Size size = texture->getContentSize();
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
    
    for(int r = 0; r < rows; ++r)
    {
        for(int c = 0; c < cols; ++c)
        {
            cocos2d::Rect rect(c*frameWidth,r*frameHeight,frameWidth,frameHeight);
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

void TimedLoopAnimation::loadAnimation(const string& name, int length, float animationInterval)
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
    sequence = AnimationSpriteSequence::loadFromRasterImage(path,4,4);
    
    sprite = Sprite::createWithSpriteFrame(sequence->frames.at(0));
    addChild(sprite,1);
    sprite->useAntiAliasTexture(false);
    
    this->setCascadeOpacityEnabled(true);
}


void PatchConAnimation::accumulate(float dx)
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
    sprite->setFlippedX(dir == Direction::leftDir);
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
    case upDir: index = animFrame; break;
    case leftDir: case rightDir: index = 8+ animFrame; break;
    case downDir: index = animFrame*4+3; break;
    }
    sprite->setSpriteFrame(sequence->frames.at(index));
}

