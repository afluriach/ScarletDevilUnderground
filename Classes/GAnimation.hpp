//
//  GAnimation.hpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#ifndef GAnimation_hpp
#define GAnimation_hpp

#include "App.h"
#include "types.h"
#include "util.h"

//The set of sprites frames used for a single animation. Since the sprite frames are
//stateless and do not contain any logic representing an animation, only one needs to exist.
//However, since SpriteFrames are a relatively lightweight wrapper (and since Textures
//are cached), it is not clear if caching these will improve performance.
//
//In order to prevent a caching system from reducing performance (by keeping textures loaded
//that are no longer used, it should use weak_ptrs. However, cocos uses it own semantics for
//managing object lifecycle so a weak_ptr will not work.
class AnimationSpriteSequence
{
public:
    static shared_ptr<AnimationSpriteSequence> loadFromRasterImage(const string& path, int cols, int rows);
    static shared_ptr<AnimationSpriteSequence> loadFromImageSequence(const string& name, int length);
	static shared_ptr<AnimationSpriteSequence> loadAgentAnimation(const string& name);

    inline AnimationSpriteSequence(Vector<SpriteFrame*> frames) : frames(frames) {}
    //Use cocos vector to manage object lifecycle.
    Vector<SpriteFrame*> frames;
};

class GAnimation : public Node
{
};

class TimedAnimation : GAnimation{
public:
    virtual void update() = 0;
};

class TimedLoopAnimation : public GAnimation {
public:
   CREATE_FUNC(TimedLoopAnimation);
   void loadAnimation(const string& name, int length, float animationInterval);
   void update();
protected:
    shared_ptr<AnimationSpriteSequence> sequence;
    float frameInterval;
    float timeInFrame = 0;
    int crntFrame = 0;
    Sprite* sprite;
};

//Sprite frames are loaded raster order. So all directions are encompassed with a single sprite frame set.
//Can go for a running effect by omitting the middle frame.
class PatchConAnimation : public GAnimation {
public:
    //One pace is 1 unit, with the middle state representing a small distance between steps.
    static constexpr float stepSize = 0.4f;
    static constexpr float midstepSize = 0.2f;
    
    CREATE_FUNC(PatchConAnimation);
    
    inline void setSpriteShader(const string& shader){
        sprite->setShader(shader);
    }
    
    void loadAnimation(const string& path);    
    void accumulate(float dx);
    
    //Reset to standing. Implicitly removes run effect.
    void reset();
    void setDirection(Direction dir);
    inline Direction getDirection()const{
        return direction;
    }
    void checkAdvanceAnimation();
protected:
    shared_ptr<AnimationSpriteSequence> sequence;

    Sprite* sprite = nullptr;
    float distanceAccumulated = 0;
    unsigned char crntFrame = 1;
    Direction direction = Direction::up;
    //Which foot to use for the first step after the animation has been standing still.
    //Represents first step that will be used next time, so it should be toggled immediately after use.
    bool firstStepIsLeft = false;
    //Which step is coming next
    bool nextStepIsLeft = true;
    
    void setFrame(int animFrame);
};



#endif /* GAnimation_hpp */
