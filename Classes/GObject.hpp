//
//  GObject.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GObject_hpp
#define GObject_hpp

class Player;
class Bullet;

class GObject
{
public:
    typedef function<GObject*( const ValueMap&) > AdapterType;
    
    static constexpr bool logCreateObjects = false;
    
    //Representation as a map object
    GObject(const ValueMap& args);
    GObject(const string& name, const SpaceVect& pos);
    
    inline virtual ~GObject() {}
    
    //Map each class name to a constructor adapter function.
    static const unordered_map<string,AdapterType> adapters;
    
    static GObject* constructByType(const string& type, const ValueMap& args );
    
    inline void setInitialVelocity(const SpaceVect&& v){
        multiInit += [=](GObject* obj) -> void{ obj->body->setVel(v);};
    }

    inline SpaceVect getPos(){
        return body->getPos();
    }

    inline void setPos(float x, float y){
        body->setPos(SpaceVect(x,y));
    }
    
    inline virtual void setDirection(Direction d) {
        if(body && d != Direction::none)
            body->setAngle(dirToPhysicsAngle(d));
    }
    
    inline SpaceVect getVel(){
        return body->getVel();
    }
    
    inline void setVel(SpaceVect v){
        body->setVel(v);
    }
    
    //Apply a force as impulse where t = frame length.
    inline void applyForceForSingleFrame(SpaceVect f){
        body->applyImpulse(f * App::secondsPerFrame);
    }
    
    inline void applyImpulse(float mag, float angle){
        SpaceVect v = SpaceVect::ray(mag,angle);
        
        body->applyImpulse(v);
    }
    
    inline string getName(){
        return name;
    }
    
    inline unsigned int getUUID(){
        return uuid;
    }

    inline static void resetObjectUUIDs(){
        nextUUID = 1;
    }

    const string name;
    const unsigned int uuid;
    
    shared_ptr<Body> body;
    shared_ptr<Body> radar;

    
    //Posiition where the object was loaded
    SpaceVect initialCenter;
    
    //Called on the first frame after it has been added, before update is called on it or any other
    //objects in the same frame
    inline void init()
    {
        multiInit(this);
    }
    
    inline void update()
    {
        multiUpdate(this);
    }
    
    //Called before adding the the object to space.
    virtual void initializeBody(GSpace& space) = 0;
    virtual void initializeRadar(GSpace& space){};
    
    //Create Node which graphically reprensets this object and adds it to Layer
    virtual void initializeGraphics(Layer* layer) = 0;
    
    Vec2 getInitialCenterPix();

    util::multifunction<void(GObject*)> multiInit;
    util::multifunction<void(GObject*)> multiUpdate;
    
      
    //Wrapper to call a method of a derived type with a GObject this.
    template<typename Derived, void (Derived::*Method)(void)>
    function<void(GObject*)> wrap()
    {
        return wrapAsBaseMethod<GObject, Derived, Method>();
    }
    
    inline virtual string getScriptVal(string field){
        log("getScriptVal: %s is not a ScriptedObject.", name.c_str());
        return "";
    }
    
    inline virtual void setScriptVal(string name, string val){
        log("setScriptVal: %s is not a ScriptedObject.", name.c_str());
    }
    inline virtual string _callScriptVal(string field, string args){
        log("_callScriptVal: %s is not a ScriptedObject.", name.c_str());
        return "";
    }

    inline virtual float getMaxSpeed() const {return 0;}
    inline virtual float getMaxAcceleration() const { return 0;}
    
private:
    static unsigned int nextUUID;
};

class ScriptedObject : virtual public GObject
{
public:
    Lua::Inst ctx;
    inline ScriptedObject(const string& script) :
    ctx(boost::lexical_cast<string>(uuid) + "_" + name)
    {
        ctx.runFile("scripts/entities/"+script+".lua");
        //Push this as a global variable in the script's context.
        ctx.setGlobal(Lua::convert<GObject*>::convertToLua(this, ctx.state), "this");

        multiInit += wrap_method(ScriptedObject,init);
        multiUpdate += wrap_method(ScriptedObject,update);
    }
    inline void init(){
        ctx.callIfExistsNoReturn("init");
    }
    inline void update(){
        ctx.callIfExistsNoReturn("update");
    }
    
    //Part of GObject API.
    inline string getScriptVal(string name){
        return ctx.getSerialized(name);
    }
    
    inline void setScriptVal(string field, string val){
        ctx.setSerialized(field,val);
    }
    
    inline string _callScriptVal(string field, string args){
        return ctx.callSerialized(field,args);
    }
};

class RadarObject : virtual public GObject
{
public:
    virtual float getRadarRadius() const = 0;
    virtual GType getRadarType() const = 0;
    
    virtual void onDetect(GObject* other) = 0;
    virtual void onEndDetect(GObject* other) = 0;


    //Create body and add it to space. This assumes BB is rectangle dimensions
    virtual inline void initializeRadar(GSpace& space)
    {
        radar = space.createCircleBody(
            initialCenter,
            getRadarRadius(),
            0.1,
            getRadarType(),
            PhysicsLayers::all,
            true,
            this
        );
    }
    
    inline RadarObject(){
        multiUpdate += wrap_method(RadarObject,updateRadarPos);
    }
    
    inline void updateRadarPos(){
        radar->setPos(body->getPos());
    }
};

class ScriptedRadar : virtual public RadarObject, virtual public ScriptedObject{
public:
	inline ScriptedRadar() {}
    inline void onDetect(GObject* other){
        ctx.callIfExistsNoReturn("onDetect",ctx.makeArgs(other));
    }
    inline void onEndDetect(GObject* other){
        ctx.callIfExistsNoReturn("onEndDetect", ctx.makeArgs(other));
    }
};

//A field of view sensor that tracks the object in front of the sensing object.
class ObjectSensor : virtual public RadarObject
{
public:
	inline ObjectSensor() {}

    //for object sensing
    virtual float getRadarRadius() const { return 2.5;}
    virtual GType getRadarType() const { return GType::objectSensor;}
    
	static const float coneHalfWidth;

    set<GObject*> inRange;
    
    inline virtual void onDetect(GObject* other)
    {
        inRange.insert(other);
    }
    inline virtual void onEndDetect(GObject* other)
    {
        inRange.erase(other);
    }

    //Find the object, if any, that is within the cone angle, selecting the one
    //that is closest to the center of the object's direction.
    GObject* getSensedObject();
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

template<typename Derived>
class RegisterInit : public virtual GObject
{
public:
    inline RegisterInit(Derived* that)
    {
        multiInit += wrap_method(Derived,init);
    }
};

template<typename Derived>
class RegisterUpdate : public virtual GObject
{
public:
    inline RegisterUpdate(Derived* that)
    {
        multiUpdate += wrap_method(Derived,update);
    }
};

class PhysicsObject : public virtual GObject
{
public:
	inline PhysicsObject() {}

    //A default of 0 signifies undefined. Using -1 to indicate static or positive for dynamic.
    virtual float getMass() const = 0;
    virtual GType getType() const = 0;
    virtual inline bool getSensor() const {return false;}
    virtual inline PhysicsLayers getLayers() const {return PhysicsLayers::ground;}
};

class RectangleBody : public virtual PhysicsObject
{
public:
    //Create body and add it to space. This assumes BB is rectangle dimensions
    virtual inline void initializeBody(GSpace& space)
    {
        body = space.createRectangleBody(
            initialCenter,
            getDimensions(),
            getMass(),
            getType(),
            getLayers(),
            getSensor(),
            this
        );
    }
    
    virtual inline SpaceVect getDimensions() const = 0;
};

class RectangleMapBody : public virtual RectangleBody
{
public:
    static SpaceVect getDimensionsFromMap(const ValueMap& arg);

    inline RectangleMapBody(const ValueMap& arg) : dim(getDimensionsFromMap(arg)) {}
    
    inline SpaceVect getDimensions() const { return dim;}

private:
    //Rectular dimensions or BB dimensions if object is not actually rectangular.
    SpaceVect dim;
};

class CircleBody : public virtual PhysicsObject
{
public:
    virtual float getRadius() const = 0;

    //Create body and add it to space. This assumes BB is rectangle dimensions
    virtual inline void initializeBody(GSpace& space)
    {
        body = space.createCircleBody(
            initialCenter,
            getRadius(),
            getMass(),
            getType(),
            getLayers(),
            getSensor(),
            this
        );
    }
};

class FrictionObject : public virtual PhysicsObject, RegisterUpdate<FrictionObject>
{
public:
    inline FrictionObject() : RegisterUpdate(this) {}

    virtual float uk() const = 0;
    void update();
};

class SpriteObject : public virtual GObject, RegisterUpdate<SpriteObject>
{
public:
    inline SpriteObject() : RegisterUpdate(this) {}
    
    inline ~SpriteObject(){        
        if(sprite)
            sprite->removeFromParent();
    }
    
    //The Z-order used by Cocos2D.
    virtual GraphicsLayer sceneLayer() const = 0;
    
    inline int sceneLayerAsInt(){
        return static_cast<int>(sceneLayer());
    };
    
    inline virtual float zoom() const {return 1;}

    void update();
    Node* sprite;
};

//Initialize graphics from a still image. Any class that uses this mixin has to implement interface to
//provide the path to the image file.
class ImageSprite : public virtual SpriteObject
{
public:
    virtual string imageSpritePath() const = 0;
    void loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest);
    inline void initializeGraphics(Layer* layer)
    {
        loadImageSprite(imageSpritePath(), sceneLayer(), layer);
    }
};

class LoopAnimationSprite : public virtual SpriteObject, RegisterUpdate<LoopAnimationSprite>
{
public:
    inline LoopAnimationSprite() : RegisterUpdate<LoopAnimationSprite>(this){
    }

    virtual string animationName() const = 0;
    virtual int animationSize() const = 0;
    virtual float animationDuration() const = 0;
    
    virtual void initializeGraphics(Layer* layer);
    inline void update()
    {
        anim->update();
    }
protected:
    TimedLoopAnimation* anim;
};

class PatchConSprite : virtual public SpriteObject, RegisterUpdate<PatchConSprite>
{
public:
    static const int pixelWidth = 32;

    inline PatchConSprite() : RegisterUpdate<PatchConSprite>(this ) {}
    virtual string imageSpritePath() const = 0;
    virtual GraphicsLayer sceneLayer() const = 0;
    
    inline float zoom() const {return App::pixelsPerTile / pixelWidth;}
    
    void initializeGraphics(Layer* layer);
    void update();
    
    void setDirection(Direction d);
    inline Direction getDirection()const{
        return animSprite->getDirection();
    }
protected:
    PatchConAnimation* animSprite;
};

class Spellcaster : public virtual GObject
{
public:
    inline Spellcaster(){
        multiUpdate += wrap_method(Spellcaster,update);
    }
    ~Spellcaster();
    void cast(shared_ptr<Spell> spell);
    void cast(const string& name, const ValueMap& args);
//For Luabridge
    void castByName(string name, const ValueMap& args);
    void update();
    void stop();
protected:
    shared_ptr<Spell> crntSpell;
};

class Enemy : public virtual GObject
{
public:
    virtual void onTouchPlayer(Player* target) = 0;
    virtual void onPlayerBulletHit(Bullet* bullet) = 0;
};

class TouchDamageEnemy : public virtual Enemy
{
public:
	inline TouchDamageEnemy() {}
    virtual void onTouchPlayer(Player* player);
};

class HitPointsEnemy : public virtual Enemy, RegisterUpdate<HitPointsEnemy>
{
    public:
        inline HitPointsEnemy(int _hp) : hp(_hp), RegisterUpdate(this) {}
    
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

#endif /* GObject_hpp */
