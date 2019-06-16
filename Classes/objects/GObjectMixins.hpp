//
//  GObjectMixins.hpp
//  Koumachika
//
//  Created by Toni on 4/15/18.
//
//

#ifndef GObjectMixins_hpp
#define GObjectMixins_hpp

#include "functional.hpp"
#include "GObject.hpp"
#include "Graphics.h"
#include "multifunction.h"

class Agent;
class PatchConAnimation;
class TimedLoopAnimation;
class Wall;

//LOGIC MIXINS

template<typename Derived>
class RegisterInit : public virtual GObject
{
public:
	inline RegisterInit(Derived* that){
        multiInit += wrap_method(Derived, init, that);
    }
};

class InteractibleObject : public virtual GObject
{
public:
	inline InteractibleObject() {}

    virtual bool canInteract(Player* p) = 0;
    virtual void interact(Player* p) = 0;
    virtual string interactionIcon(Player* p) = 0;
};

class ActivateableObject : public virtual GObject
{
public:
	inline ActivateableObject() {}

	virtual void activate() = 0;
	virtual void deactivate() = 0;
};

class DialogEntity : public InteractibleObject
{
public:
	inline DialogEntity() {}

    virtual bool isDialogAvailable() = 0;
    virtual string getDialog() = 0;
	inline virtual void onDialogEnd() {}

    inline virtual bool canInteract(Player* p){
        return isDialogAvailable();
    }
    virtual void interact(Player* p);

    inline virtual string interactionIcon(Player* p){
        return "sprites/ui/dialog.png";
    }
};

class DialogImpl : public DialogEntity
{
public:
	DialogImpl(const ValueMap& args);

	inline virtual bool isDialogAvailable() { return !dialogName.empty(); }
	inline virtual string getDialog() { return "dialogs/"+dialogName; }
protected:
	string dialogName;
};

//END LOGIC

//PHYSICS MIXINS

class RectangleBody : public virtual GObject
{
public:
	inline RectangleBody() : dim(SpaceVect(1, 1)) {}
	inline RectangleBody(SpaceVect dim) : dim(dim) {}
	RectangleBody(const ValueMap& arg);

    //Create body and add it to space. This assumes BB is rectangle dimensions
    virtual void initializeBody(GSpace& space);    

	inline virtual SpaceFloat getRadius() const { return dim.getMax(); }

	const SpaceVect dim;
};

class CircleBody : public virtual GObject
{
public:
	inline CircleBody() {}

    //Create body and add it to space. This assumes BB is rectangle dimensions
    virtual void initializeBody(GSpace& space);
};

class MassImpl : public virtual GObject
{
public:
	inline MassImpl(SpaceFloat mass) : mass(mass) {}
	
	virtual inline SpaceFloat getMass() const { return mass; }

	const SpaceFloat mass;
};

//Allow the speed of an object to be supplied as construction parameter.
class MaxSpeedImpl : virtual public GObject
{
public:
	inline MaxSpeedImpl(SpaceFloat speed) : speed(speed) {}
	virtual inline SpaceFloat getMaxSpeed() const { return speed; }
protected:
	SpaceFloat speed;
};

class ParametricMotion :
	virtual public GObject,
	public RegisterInit<ParametricMotion>
{
public:
	ParametricMotion(parametric_space_function f, SpaceFloat start = 0.0, SpaceFloat scale = 1.0);

	void init();
	void _update();
protected:
	const SpaceFloat scale;
	SpaceFloat t = 0.0;
	SpaceVect origin;
	parametric_space_function f;
};

//END PHYSICS

//GRAPHICS MIXINS

class NoSprite : public virtual GObject
{
public:
	inline NoSprite() {}

	inline virtual void initializeGraphics() {}
};


//Initialize graphics from a still image. Any class that uses this mixin has to implement interface to
//provide the path to the image file.
class ImageSprite : public virtual GObject
{
public:
    inline ImageSprite() {
		rotateSprite = true;
	}

	virtual void initializeGraphics();
	virtual string imageSpritePath() const = 0;

	void loadImageSprite(const string& resPath, GraphicsLayer sceneLayer);
};

class LoopAnimationSprite : public virtual GObject
{
public:
	inline LoopAnimationSprite() {}

    virtual string animationName() const = 0;
    virtual int animationSize() const = 0;
    virtual float animationDuration() const = 0;
    
    virtual void initializeGraphics();
protected:
    SpriteID animID = 0;
};

class PatchConSprite : virtual public GObject, RegisterInit<PatchConSprite>
{
public:
	static constexpr SpaceFloat stepSize = 0.4;
	static constexpr SpaceFloat midstepSize = 0.2;

    PatchConSprite(const ValueMap& args);
	PatchConSprite(Direction startingDirection = Direction::up);

    virtual string imageSpritePath() const = 0;
    
    float zoom() const;
	virtual inline int pixelWidth() const { return 32; }
	virtual bool isAgentAnimation() const { return false; }

    void initializeGraphics();
    void init();
    void _update();
    
    void setSprite(const string& name);
	void setSprite(const string& name, bool agentAnimation);
    
    virtual void setAngle(SpaceFloat a);
    virtual void setDirection(Direction d);

	bool accumulate(SpaceFloat dx);
	bool checkAdvanceAnimation();
	void reset();
protected:
	SpaceFloat accumulator = 0.0;
    Direction startingDirection = Direction::down;
	int crntFrame = 1;
	bool firstStepIsLeft = false;
	//Which step is coming next
	bool nextStepIsLeft = true;
};

class RadialLightObject : virtual public GObject, public RegisterInit<RadialLightObject>
{
public:
	RadialLightObject();

	virtual CircleLightArea getLightSource() const = 0;

	void init();
};

class SpriteLightObject : virtual public GObject, public RegisterInit<SpriteLightObject>
{
public:
	SpriteLightObject();

	virtual SpriteLightArea getLightSource() const = 0;

	void init();
};

//END GRAPHICS

//BEGIN AUDIO
class AudioSourceObject : virtual public GObject
{
public:
	inline AudioSourceObject() {}

	void _update();
	ALuint playSoundSpatial(const string& path, float volume = 1.0f, bool loop = false);
	void stopSound(ALuint source);
	bool isSourceActive(ALuint source);
protected:
	list<ALuint> sources;
};
//END AUDIO

#endif /* GObjectMixins_hpp */
