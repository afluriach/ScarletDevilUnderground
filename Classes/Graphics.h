//
//  Graphics.h
//  Koumachika
//
//  Created by Toni on 10/7/15.
//
//

#ifndef Graphics_h
#define Graphics_h

#include "types.h"

class Cursor : public Node
{
public:
	static const Color4F colors[6];

    virtual void onEnter();
    virtual void update(float dt);
    void reset();
protected:
    virtual void drawShape() = 0;
    //The length of time for the diamond to expand or shrink fully.
    const float interval = 0.5;
    //Base size of shape.
    const int halfSize = 20;

	int scaledSize();
        
    DrawNode * drawNode;
    bool expanding=false;
    float scale = 1;
    int crntColor=0;
};

class DiamondCursor : public Cursor
{
public:
	//Y scale, with is also y/x ratio
	static const float ratio;
    virtual void onEnter();
protected:
    virtual void drawShape();
};

class DownTriangleCursor : public Cursor
{
public:
    virtual void drawShape();
};

Action* flickerAction(float interval, float length, float opacity);
Action* flickerTintAction(float interval, float length, Color3B tint);
Action* tintTo(Color3B tint, float length);
Action* motionBlurStretch(float duration, float angle, float opacity, float scale);
FiniteTimeAction* pitfallShrinkAction();

Sprite* loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest, const Vec2& pos, float zoom);

Label* createTextLabel(const string& text, int size);

cocos2d::CCSize getScreenSize();

#endif /* Graphics_h */
