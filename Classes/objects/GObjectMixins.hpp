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

class DialogEntity : public virtual GObject
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
	inline CircleBody(SpaceFloat radius) : radius(radius) {}

    //Create body and add it to space. This assumes BB is rectangle dimensions
    virtual void initializeBody(GSpace& space);
	inline virtual SpaceFloat getRadius() const { return radius; }

protected:
	const SpaceFloat radius;
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

//END PHYSICS

//GRAPHICS MIXINS

class PatchConSprite : virtual public GObject, RegisterInit<PatchConSprite>
{
public:
	static constexpr SpaceFloat stepSize = 0.4;
	static constexpr SpaceFloat midstepSize = 0.2;

    PatchConSprite(const ValueMap& args);
	PatchConSprite(Direction startingDirection = Direction::up);

    void initializeGraphics();
    void init();
    void _update();
    
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
