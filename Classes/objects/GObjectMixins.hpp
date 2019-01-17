//
//  GObjectMixins.hpp
//  Koumachika
//
//  Created by Toni on 4/15/18.
//
//

#ifndef GObjectMixins_hpp
#define GObjectMixins_hpp

#include "Attributes.hpp"
#include "functional.hpp"
#include "GObject.hpp"
#include "multifunction.h"
#include "util.h"

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

template<typename Derived>
class RegisterUpdate : public virtual GObject
{
public:
	inline RegisterUpdate(Derived* that){
        multiUpdate += wrap_method(Derived, update, that);
    }
};

class InteractibleObject : public virtual GObject
{
public:
    virtual bool canInteract() = 0;
    virtual void interact() = 0;
    virtual string interactionIcon() = 0;
};

class ActivateableObject : public virtual GObject
{
public:
	virtual void activate() = 0;
	virtual void deactivate() = 0;
};

class DialogEntity : public InteractibleObject
{
    virtual bool isDialogAvailable() = 0;
    virtual string getDialog() = 0;

    inline virtual bool canInteract(){
        return isDialogAvailable();
    }
    virtual void interact();

    inline virtual string interactionIcon(){
        return "sprites/ui/dialog.png";
    }
};

//END LOGIC

//PHYSICS MIXINS

class RectangleBody : public virtual GObject
{
public:
	inline RectangleBody(){}
    //Create body and add it to space. This assumes BB is rectangle dimensions
    virtual void initializeBody(GSpace& space);    
    virtual SpaceFloat getMomentOfInertia() const;

	inline virtual SpaceFloat getRadius() const { return max(getDimensions().x, getDimensions().y); }

	virtual CCRect getBoundingBox();
};

class RectangleMapBody : public virtual RectangleBody
{
public:
    static SpaceVect getDimensionsFromMap(const ValueMap& arg);

    inline RectangleMapBody(const ValueMap& arg) : dim(getDimensionsFromMap(arg)) {}
    
    inline SpaceVect getDimensions() const { return dim;}
	inline virtual SpaceFloat getRadius() const { return max(dim.x, dim.y); }

private:
    //Rectular dimensions or BB dimensions if object is not actually rectangular.
    SpaceVect dim;
};

class CircleBody : public virtual GObject
{
public:
    virtual SpaceFloat getRadius() const = 0;
    virtual SpaceFloat getMomentOfInertia() const;

    //Create body and add it to space. This assumes BB is rectangle dimensions
    virtual void initializeBody(GSpace& space);

	virtual CCRect getBoundingBox();
	virtual SpaceVect getDimensions() const;
};

class FrictionObject : public virtual GObject, RegisterUpdate<FrictionObject>
{
public:
    inline FrictionObject() : RegisterUpdate(this) {}

    virtual SpaceFloat uk() const = 0;
    void update();
};

class MassImpl : public virtual GObject
{
public:
	inline MassImpl(SpaceFloat mass) : mass(mass) {}
	
	virtual inline SpaceFloat getMass() const { return mass; }

	const SpaceFloat mass;
};

//Object will automatically have its velocity set on init(), according to
//its [facing] angle. Uses polymorphic getter getMaxSpeed().
class DirectionalLaunch : virtual public GObject, public RegisterInit<DirectionalLaunch>
{
public:
	inline DirectionalLaunch() : RegisterInit<DirectionalLaunch>(this) {}
	void init();
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
class ImageSprite : public virtual GObject, RegisterUpdate<ImageSprite>
{
public:
    ImageSprite() : RegisterUpdate<ImageSprite>(this) {}

    virtual string imageSpritePath() const = 0;
    void loadImageSprite(const string& resPath, GraphicsLayer sceneLayer);
    void initializeGraphics();
    void update();
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
    unsigned int animID = 0;
};

class PatchConSprite : virtual public GObject, RegisterInit<PatchConSprite>, RegisterUpdate<PatchConSprite>
{
public:
	static constexpr SpaceFloat stepSize = 0.4;
	static constexpr SpaceFloat midstepSize = 0.2;

    PatchConSprite(const ValueMap& args);
    virtual string imageSpritePath() const = 0;
    
    float zoom() const;
	virtual inline int pixelWidth() const { return 32; }
	virtual bool isAgentAnimation() const { return false; }

    void initializeGraphics();
    void init();
    void update();
    
    void setSprite(const string& name);
    
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

//END GRAPHICS

#endif /* GObjectMixins_hpp */
