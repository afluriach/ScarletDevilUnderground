//
//  Graphics.h
//  FlansBasement
//
//  Created by Toni on 10/7/15.
//
//

#ifndef Graphics_h
#define Graphics_h

class Cursor : public Node
{
public:
    inline virtual void onEnter()
    {
        Node::onEnter();
        scheduleUpdate();
        
        
        drawNode = DrawNode::create();
        addChild(drawNode);
        
        drawShape();
    }
    
    inline virtual void update(float dt)
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
    inline void reset()
    {
        scale = 1;
        crntColor = 0;
        expanding = false;
        drawShape();
    }
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
    inline virtual void onEnter()
    {
        Cursor::onEnter();
        drawNode->setRotation(45);
        setScaleY(ratio);
    }
    CREATE_FUNC(DiamondCursor);
protected:
    //Y scale.
    const float ratio = 1.5;

    virtual inline void drawShape()
    {
        drawNode->clear();
        drawNode->drawSolidRect(Vec2(-halfSize,-halfSize), Vec2(halfSize,halfSize), colors[crntColor]);
    }
};

class DownTriangleCursor : public Cursor
{
public:
    CREATE_FUNC(DownTriangleCursor);
protected:
    virtual inline void drawShape()
    {
        drawNode->clear();
        
        cocos2d::Vec2 left(-halfSize,halfSize);
        cocos2d::Vec2 right(halfSize,halfSize);
        cocos2d::Vec2 bottom(0,-halfSize);
        
        drawNode->drawTriangle(left,right,bottom,colors[crntColor]);
    }
};

cocos2d::Size getScreenSize();

#endif /* Graphics_h */
