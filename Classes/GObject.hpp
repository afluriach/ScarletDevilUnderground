//
//  GObject.hpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GObject_hpp
#define GObject_hpp

class GObject
{
public:
    typedef function<GObject*( const ValueMap&) > AdapterType;
    
    //Representation as a map object
    GObject(const ValueMap& args);
    GObject(const string& name, const SpaceVect& pos);
    
    inline virtual ~GObject() {}
    
    //Map each class name to a constructor adapter function.
    static const map<string,AdapterType> adapters;
    
    static GObject* constructByType(const string& type, const ValueMap& args );
    
    static ValueMap makeValueMapArg(const Vec2& pos, const map<string,string>& props);
    
    inline void setInitialVelocity(const SpaceVect&& v){
        multiInit += [=]() -> void{ body->setVel(v);};
    }

    inline void setPos(float x, float y){
        body->setPos(SpaceVect(x,y));
    }
    
    inline void applyImpulse(float mag, float angle){
        SpaceVect v = SpaceVect::ray(mag,angle);
        
        body->applyImpulse(v);
    }
    
    inline unsigned int getUUID(){
        return uuid;
    }

    const string name;
    const unsigned int uuid;
    
    shared_ptr<Body> body;
    
    //Posiition where the object was loaded
    SpaceVect initialCenter;
    
    //Called on the first frame after it has been added, before update is called on it or any other
    //objects in the same frame
    inline void init()
    {
        multiInit();
    }
    
    inline void update()
    {
        multiUpdate();
    }
    
    //Called before adding the the object to space.
    virtual shared_ptr<Body>initializeBody(Space& space) = 0;
    
    //Create Node which graphically reprensets this object and adds it to Layer
    virtual void initializeGraphics(Layer* layer) = 0;
    
    Vec2 getInitialCenterPix();

    util::multifunction<void()> multiInit;
    util::multifunction<void()> multiUpdate;
private:
    static unsigned int nextUUID;
};

template<typename Derived>
class RegisterInit : public virtual GObject
{
public:
    inline RegisterInit(Derived* that)
    {
        multiInit += bind(&Derived::init, that);
    }
};

template<typename Derived>
class RegisterUpdate : public virtual GObject
{
public:
    inline RegisterUpdate(Derived* that)
    {
        multiUpdate += bind(&Derived::update, that);
    }
};

class PhysicsObject : public virtual GObject
{
public:
    //A default of 0 signifies undefined. Using -1 to indicate static or positive for dynamic.
    virtual float getMass() const = 0;
    virtual GType getType() const = 0;
    virtual inline bool getSensor() const {return false;}
    virtual inline int getLayers() const {return PhysicsLayers::groundLayer;}
};

class RectangleBody : public virtual PhysicsObject
{
public:
    //Create body and add it to space. This assumes BB is rectangle dimensions
    virtual inline shared_ptr<Body> initializeBody(Space& space)
    {
        body = GSpace::createRectangleBody(
            space,
            initialCenter,
            getDimensions(),
            getMass(),
            getType(),
            getLayers(),
            getSensor(),
            this
        );
        return body;
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
    virtual inline shared_ptr<Body> initializeBody(Space& space)
    {
        body = GSpace::createCircleBody(
            space,
            initialCenter,
            getRadius(),
            getMass(),
            getType(),
            getLayers(),
            getSensor(),
            this
        );
        return body;
    }
};

class SpriteObject : public virtual GObject, RegisterUpdate<SpriteObject>
{
public:
    inline SpriteObject() : RegisterUpdate(this) {}
    
    inline ~SpriteObject(){        
        if(sprite)
            sprite->removeFromParent();
    }
    
    virtual GraphicsLayer sceneLayer() const = 0;
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
protected:
    PatchConAnimation* animSprite;
};

class Spellcaster : public virtual GObject
{
public:
    inline Spellcaster(){
        multiUpdate += bind(&Spellcaster::update, this);
    }
    ~Spellcaster();
    void cast(shared_ptr<Spell> spell);
    void cast(const string& name);
//For Luabridge
    void castByName(string name);
    void update();
    void stop();
protected:
    shared_ptr<Spell> crntSpell;
};

#endif /* GObject_hpp */
