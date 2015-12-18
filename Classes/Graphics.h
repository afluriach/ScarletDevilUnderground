//
//  Graphics.h
//  Koumachika
//
//  Created by Toni on 10/7/15.
//
//

#ifndef Graphics_h
#define Graphics_h

class Cursor : public Node
{
public:
    virtual void onEnter();
    virtual void update(float dt);
    void reset();
protected:
    virtual void drawShape() = 0;
    //The length of time for the diamond to expand or shrink fully.
    const float interval = 0.5;
    //Base size of shape.
    const int halfSize = 20;
    
    Color4F colors[6] = {
        Color4F(1,0,0,1),
        Color4F(0.8,0.4,0,1),
        Color4F(0.8,0.8,0,1),
        Color4F(0,0.8,0,1),
        Color4F(0,0,1,1),
        Color4F(0.6,0,0.6,1)
    };
    
    DrawNode * drawNode;
    bool expanding=false;
    float scale = 1;
    int crntColor=0;
};

class DiamondCursor : public Cursor
{
public:
    virtual void onEnter();
    CREATE_FUNC(DiamondCursor);
protected:
    //Y scale.
    const float ratio = 1.5;

    virtual void drawShape();
};

class DownTriangleCursor : public Cursor
{
public:
    CREATE_FUNC(DownTriangleCursor);
protected:
    virtual void drawShape();
};

Action* flickerAction(float interval, float length, float opacity);

cocos2d::Size getScreenSize();

#endif /* Graphics_h */
