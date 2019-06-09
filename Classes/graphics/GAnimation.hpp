//
//  GAnimation.hpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#ifndef GAnimation_hpp
#define GAnimation_hpp

#include "types.h"

SpriteFrame* loadSpriteFrame(const string& path);

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
    static AnimationSpriteSequence loadFromRasterImage(const string& path, int cols, int rows);
	static array<AnimationSpriteSequence, 4> loadPatchconSpriteSheet(const string& path);
	static array<AnimationSpriteSequence, 4> loadAgentAnimation(const string& path);

	AnimationSpriteSequence();
	AnimationSpriteSequence(Vector<SpriteFrame*> frames);
	AnimationSpriteSequence(Vector<SpriteFrame*>::iterator begin, Vector<SpriteFrame*>::iterator end);

    Vector<SpriteFrame*> frames;
};

class GAnimation : public Node
{
public:
    virtual void update() = 0;
};

class TimedLoopAnimation : public GAnimation {
public:
   void loadAnimation(const string& name, int length, SpaceFloat animationInterval);
   virtual void update();
protected:
    AnimationSpriteSequence sequence;
    SpaceFloat frameInterval;
    SpaceFloat timeInFrame = 0.0;
    int crntFrame = 0;
    Sprite* sprite = nullptr;
};

//Sprite frames are loaded raster order. So all directions are encompassed with a single sprite frame set.
//Can go for a running effect by omitting the middle frame.
class PatchConAnimation : public GAnimation {
public:
    //One pace is 1 unit, with the middle state representing a small distance between steps.
    static constexpr SpaceFloat stepSize = 0.4;
    static constexpr SpaceFloat midstepSize = 0.2;
    
	void setSpriteShader(const string& shader);
    void loadAnimation(const string& path, bool agentAnimation);    

	void setFrame(int animFrame);
    void setDirection(Direction dir);
	Direction getDirection()const;

	inline virtual void update() {}
protected:
	bool useFlipX = false;

	array<AnimationSpriteSequence, 4> walkAnimations;

    Sprite* sprite = nullptr;
    SpaceFloat distanceAccumulated = 0.0;
    unsigned char crntFrame = 1;
    Direction direction = Direction::up;
};



#endif /* GAnimation_hpp */
