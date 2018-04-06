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
        Color4F(1.0f,0.0f,0.0f,1.0f),
        Color4F(0.8f,0.4f,0.0f,1.0f),
        Color4F(0.8f,0.8f,0.0f,1.0f),
        Color4F(0.0f,0.8f,0.0f,1.0f),
        Color4F(0.0f,0.0f,1.0f,1.0f),
        Color4F(0.6f,0.0f,0.6f,1.0f)
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

class InvertedSprite : public Sprite
{
public:
    inline static InvertedSprite* create(const std::string& filename){
        InvertedSprite *sprite = new (std::nothrow) InvertedSprite();
        if (sprite && sprite->initWithFile(filename))
        {
            sprite->autorelease();
            return sprite;
        }
        CC_SAFE_DELETE(sprite);
        return nullptr;
    }

    virtual inline string getShaderName() const{
        return "inverted";
    }
};

class HueShiftLeftSprite : public Sprite
{
public:
    inline static HueShiftLeftSprite* create(const std::string& filename){
        HueShiftLeftSprite *sprite = new (std::nothrow) HueShiftLeftSprite();
        if (sprite && sprite->initWithFile(filename))
        {
            sprite->autorelease();
            return sprite;
        }
        CC_SAFE_DELETE(sprite);
        return nullptr;
    }
    virtual inline string getShaderName() const{
        return "hue_shift_left";
    }
};

class HueShiftRightSprite : public Sprite
{
public:
    inline static HueShiftRightSprite* create(const std::string& filename){
        HueShiftRightSprite *sprite = new (std::nothrow) HueShiftRightSprite();
        if (sprite && sprite->initWithFile(filename))
        {
            sprite->autorelease();
            return sprite;
        }
        CC_SAFE_DELETE(sprite);
        return nullptr;
    }

    virtual inline string getShaderName() const{
        return "hue_shift_right";
    }
};

Action* flickerAction(float interval, float length, float opacity);

cocos2d::Size getScreenSize();

#endif /* Graphics_h */
