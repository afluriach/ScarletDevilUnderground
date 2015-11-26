//
//  GAnimation.hpp
//  FlansBasement
//
//  Created by Toni on 11/24/15.
//
//

#ifndef GAnimation_hpp
#define GAnimation_hpp

//The set of sprites frames used for a single animation. Since the sprite frames are
//stateless and do not contain any logic representing an animation, only one needs to exist.
//However, since SpriteFrames are a relatively lightweight wrapper (and since Textures
//are cached), it is not clear if caching these will improve performance.
//
//In order to prevent a caching system from reducing performance (by keeping textures loaded
//that are no longer used, it should use weak_ptrs. However, cocos uses it own semantics for
//managing object lifecycle so a std::weak_ptr will not work.
class AnimationSpriteSequence
{
public:
    static shared_ptr<AnimationSpriteSequence> loadFromRasterImage(const string& path, int cols, int rows);
    
    inline AnimationSpriteSequence(cocos2d::Vector<SpriteFrame*> frames) : frames(frames) {}
    //Use cocos vector to manage object lifecycle.
    cocos2d::Vector<SpriteFrame*> frames;
};

class GAnimation : public cocos2d::Node
{
};

//Sprite frames are loaded raster order. So all directions are encompassed with a single sprite frame set.
//Can go for a running effect by omitting the middle frame.
class PatchConAnimation : public GAnimation {
public:
    static const int pixelWidth = 32;
    static constexpr int zoom = App::pixelsPerTile / pixelWidth;
    
    //One pace is 1 unit, with the middle state representing a small distance between steps.
    static constexpr float stepSize = 0.3;
    static constexpr float midstepSize = 0.15;
    
    CREATE_FUNC(PatchConAnimation);
    
    inline void loadAnimation(const string& path)
    {
        sequence = AnimationSpriteSequence::loadFromRasterImage(path,4,4);
        
        sprite = Sprite::createWithSpriteFrame(sequence->frames.at(0));
        sprite->setScale(zoom);
        addChild(sprite,1);
        sprite->useAntiAliasTexture(false);
    }
    
    void accumulate(float dx);
    
    //Reset to standing. Implicitly removes run effect.
    void reset();
    void setDirection(Direction dir);
    void checkAdvanceAnimation();
protected:
    shared_ptr<AnimationSpriteSequence> sequence;

    Sprite* sprite;
    float distanceAccumulated = 0;
    unsigned char crntFrame = 1;
    Direction direction = Direction::upDir;
    //Which foot to use for the first step after the animation has been standing still.
    //Represents first step that will be used next time, so it should be toggled immediately after use.
    bool firstStepIsLeft = false;
    //Which step is coming next
    bool nextStepIsLeft = true;
    
    void setFrame(int animFrame);
};



#endif /* GAnimation_hpp */
