//
//  GAnimation.cpp
//  FlansBasement
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "GAnimation.hpp"

shared_ptr<AnimationSpriteSequence> AnimationSpriteSequence::loadFromRasterImage(const string& path, int cols, int rows)
{
    Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(path);
    
    if(!texture){
        log("AnimationSpriteSequence: texture %s not loaded!", path.c_str());
        return shared_ptr<AnimationSpriteSequence>(nullptr);
    }
    
    cocos2d::Vector<SpriteFrame*> frames;
    
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

