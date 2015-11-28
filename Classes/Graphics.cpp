//
//  Graphics.cpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#include "Prefix.h"

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

