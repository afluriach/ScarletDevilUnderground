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
    virtual float getMomentOfInertia() const;

	inline virtual float getRadius() const { return max(getDimensions().x, getDimensions().y); }

};

class RectangleMapBody : public virtual RectangleBody
{
public:
    static SpaceVect getDimensionsFromMap(const ValueMap& arg);

    inline RectangleMapBody(const ValueMap& arg) : dim(getDimensionsFromMap(arg)) {}
    
    inline SpaceVect getDimensions() const { return dim;}
	inline virtual float getRadius() const { return max(dim.x, dim.y); }

private:
    //Rectular dimensions or BB dimensions if object is not actually rectangular.
    SpaceVect dim;
};

class CircleBody : public virtual GObject
{
public:
    virtual float getRadius() const = 0;
    virtual float getMomentOfInertia() const;

    //Create body and add it to space. This assumes BB is rectangle dimensions
    virtual void initializeBody(GSpace& space);
};

class FrictionObject : public virtual GObject, RegisterUpdate<FrictionObject>
{
public:
    inline FrictionObject() : RegisterUpdate(this) {}

    virtual float uk() const = 0;
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

//END PHYSICS

//GRAPHICS MIXINS

//Initialize graphics from a still image. Any class that uses this mixin has to implement interface to
//provide the path to the image file.
class ImageSprite : public virtual GObject, RegisterUpdate<ImageSprite>
{
public:
    ImageSprite() : RegisterUpdate<ImageSprite>(this) {}

    virtual string imageSpritePath() const = 0;
    void loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest);
    void initializeGraphics(Layer* layer);
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
    
    virtual void initializeGraphics(Layer* layer);
    void update();
protected:
    TimedLoopAnimation* anim;
};

class PatchConSprite : virtual public GObject, RegisterInit<PatchConSprite>, RegisterUpdate<PatchConSprite>
{
public:
    static const int pixelWidth = 32;

    PatchConSprite(const ValueMap& args);
    virtual string imageSpritePath() const = 0;
    virtual GraphicsLayer sceneLayer() const = 0;
    
    float zoom() const;
    
    void initializeGraphics(Layer* layer);
    void init();
    void update();
    
    void setSprite(const string& name);
    virtual void setSpriteShader(const string& shaderName);
    
    virtual void setAngle(float a);
    void setDirection(Direction d);
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
	inline virtual void onPlayerBulletHit(Bullet* bullet) {}
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

class HitPointsEnemy : public virtual Enemy, RegisterUpdate<HitPointsEnemy>
{
    public:
        inline HitPointsEnemy(int _hp) : hp(_hp), RegisterUpdate<HitPointsEnemy>(this) {}
    
        void update();
    
        void hit(int damage);
    private:
        int hp;
};

class PlayerBulletDamage : public virtual HitPointsEnemy
{
public:
	inline PlayerBulletDamage() {}
	virtual void onPlayerBulletHit(Bullet* bullet);
};

//END ENEMY

//TYPE MIXINS

class Bullet : virtual public GObject
{
public:
	inline Bullet() {}

	virtual inline bool getSensor() const { return true; }
	virtual inline float getMass() const { return 0.1f; }
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }
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
	inline EnemyBullet() {}

	virtual inline GType getType() const { return GType::enemyBullet; }
};


//END TYPE MIXINS

#endif /* GObjectMixins_hpp */
