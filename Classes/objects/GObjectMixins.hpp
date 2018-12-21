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

class PatchConAnimation;
class TimedLoopAnimation;

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
    virtual inline SpaceVect getDimensions() const = 0;
    virtual SpaceFloat getMomentOfInertia() const;

	inline virtual SpaceFloat getRadius() const { return max(getDimensions().x, getDimensions().y); }

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
};

class FrictionObject : public virtual GObject, RegisterUpdate<FrictionObject>
{
public:
    inline FrictionObject() : RegisterUpdate(this) {}

    virtual SpaceFloat uk() const = 0;
    void update();
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

	inline virtual void initializeGraphics(SpaceLayer* layer) {}
};


//Initialize graphics from a still image. Any class that uses this mixin has to implement interface to
//provide the path to the image file.
class ImageSprite : public virtual GObject, RegisterUpdate<ImageSprite>
{
public:
    ImageSprite() : RegisterUpdate<ImageSprite>(this) {}

    virtual string imageSpritePath() const = 0;
    void loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest);
    void initializeGraphics(SpaceLayer* layer);
    void update();
    virtual void setSpriteShader(const string& shaderName);
};

class LoopAnimationSprite : public virtual GObject, RegisterUpdate<LoopAnimationSprite>
{
public:
    inline LoopAnimationSprite() : RegisterUpdate<LoopAnimationSprite>(this){
    }

    virtual string animationName() const = 0;
    virtual int animationSize() const = 0;
    virtual float animationDuration() const = 0;
    
    virtual void initializeGraphics(SpaceLayer* layer);
    void update();
protected:
    TimedLoopAnimation* anim;
};

class PatchConSprite : virtual public GObject, RegisterInit<PatchConSprite>, RegisterUpdate<PatchConSprite>
{
public:
    PatchConSprite(const ValueMap& args);
    virtual string imageSpritePath() const = 0;
    
    float zoom() const;
	virtual inline int pixelWidth() const { return 32; }

    void initializeGraphics(SpaceLayer* layer);
    void init();
    void update();
    
    void setSprite(const string& name);
    virtual void setSpriteShader(const string& shaderName);
    
    virtual void setAngle(SpaceFloat a);
    virtual void setDirection(Direction d);
	void updateDirection(Direction d);
    Direction getDirection()const;
protected:
    PatchConAnimation* animSprite;
    Direction startingDirection = Direction::down;
};

//END GRAPHICS

//ENEMY MIXINS

class Enemy : virtual public GObject
{
public:
	inline Enemy() {}
	inline virtual void onTouchPlayer(Player* target) {}
	inline virtual void endTouchPlayer() {}
	void runDamageFlicker();
};

class TouchDamageEnemy : public virtual Enemy, RegisterUpdate<TouchDamageEnemy>
{
public:
	inline TouchDamageEnemy() : RegisterUpdate<TouchDamageEnemy>(this) {}
    virtual void onTouchPlayer(Player* player);
	virtual void endTouchPlayer();
	void update();
protected:
	Player * hitTarget = nullptr;
};

//END ENEMY

//TYPE MIXINS

class Bullet : virtual public GObject
{
public:
	inline Bullet() {}

	virtual inline bool getSensor() const { return true; }
	virtual inline SpaceFloat getMass() const { return 0.1; }
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::ground; }

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	//Interface
	virtual inline AttributeMap getAttributeEffect() const { return AttributeMap(); }
	virtual inline shared_ptr<MagicEffect> getMagicEffect(gobject_ref target) { return nullptr; }
protected:
	int damage = 1;
};

class PlayerBullet : virtual public GObject, public Bullet
{
public:
	inline PlayerBullet() {}

	virtual inline GType getType() const { return GType::playerBullet; }
};

class EnemyBullet : virtual public GObject, public Bullet
{
public:
	inline EnemyBullet() : EnemyBullet(0.0,0) {}
	EnemyBullet(SpaceFloat grazeRadius, int grazeBonus);

	//The bullet's graze "radar" has collided with Player.
	void onGrazeTouch(object_ref<Player> obj);
	//Effect is applied after the graze "radar" loses contact.
	void onGrazeCleared(object_ref<Player> obj);
	void invalidateGraze();

	virtual void initializeRadar(GSpace& space);

	virtual inline GType getType() const { return GType::enemyBullet; }
protected:
	object_ref<Player> grazeTarget;
	bool grazeValid = true;
	const SpaceFloat grazeRadius;
	const int grazeBonus;
};

//END TYPE MIXINS

#endif /* GObjectMixins_hpp */
