//
//  Graphics.cpp
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#include "Prefix.h"

#include "Graphics.h"

//Qualify Size for ambiguity.
cocos2d::Size getScreenSize()
{
    return Director::getInstance()->getVisibleSize();
}

void Cursor::onEnter()
{
    Node::onEnter();
    scheduleUpdate();
    
    
    drawNode = DrawNode::create();
    addChild(drawNode);
    
    drawShape();
}

void Cursor::update(float dt)
{
    float scaleDelta = dt / interval;
    
    if(expanding)
    {
        scale += scaleDelta;
        
        if(scale >= 1)
        {
            expanding = false;
            scale = 1 - (scale -1);
        }
    }
    else
    {
        scale -= scaleDelta;
        
        if(scale <= 0)
        {
            expanding = true;
            scale = 0 - scale;
            
            ++crntColor;
            crntColor %= 6;
            
            drawShape();
        }
    }
    setScaleX(scale);
}
void Cursor::reset()
{
    scale = 1;
    crntColor = 0;
    expanding = false;
    drawShape();
}

void DiamondCursor::onEnter()
{
    Cursor::onEnter();
    drawNode->setRotation(45);
    setScaleY(ratio);
}

void DiamondCursor::drawShape()
{
    drawNode->clear();
    drawNode->drawSolidRect(Vec2(-halfSize,-halfSize), Vec2(halfSize,halfSize), colors[crntColor]);
}

void DownTriangleCursor::drawShape()
{
    drawNode->clear();
    
    Vec2 left(-halfSize,halfSize);
    Vec2 right(halfSize,halfSize);
    Vec2 bottom(0,-halfSize);
    
    drawNode->drawTriangle(left,right,bottom,colors[crntColor]);
}

Action* flickerAction(float interval, float length, float opacity)
{
    int nCycles = length / interval;
    
    Sequence* flicker = Sequence::createWithTwoActions(FadeTo::create(interval/2, opacity*255), FadeTo::create(interval/2, 255));
    Repeat* loop = Repeat::create(flicker, nCycles);
    
    return loop;
}

Action* flickerTintAction(float interval, float length, Color3B tint)
{
    int nCycles = length / interval;
    
    Sequence* flicker = Sequence::createWithTwoActions(
        TintTo::create(interval/2, tint.r, tint.g,tint.b),
        TintTo::create(interval/2, 255,255,255)
    );
    
    Repeat* loop = Repeat::create(flicker, nCycles);
    
    return loop;
}

Action* motionBlurStretch(float duration, float angle, float opacity, float scale)
{
//    float scaleX = 1 + cos(angle)*(scale-1);
//    float scaleY = 1 + sin(angle)*(scale-1);
//
//    Sequence* sequence = Sequence::create(
//        ScaleBy::create(0.0f, scaleX, scaleY),
//        FadeTo::create(0.0f,opacity*255),
//        DelayTime::create(duration),
//        ScaleBy::create(0.0f, 1.0f/scaleX, 1.0f/scaleY),
//        FadeTo::create(0.0f,255),
//        nullptr
//   );

    Sequence* sequence = Sequence::create(
        FadeTo::create(0.0f,opacity*255),
        DelayTime::create(duration),
        FadeTo::create(0.0f,255),
        nullptr
   );
    
    return sequence;
}
