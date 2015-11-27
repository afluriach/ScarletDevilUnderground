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
    
    virtual ~GObject() {}
    
    //Map each class name to a constructor adapter function.
    static const map<string,AdapterType> adapters;
    
    static GObject* constructByType(const string& type, const ValueMap& args );
    
    static ValueMap makeValueMapArg(const Vec2& pos, const map<string,string>& props);

    const string name;
    
    shared_ptr<cp::Body> body;
    
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
    virtual shared_ptr<cp::Body>initializeBody(cp::Space& space) = 0;
    
    //Create Node which graphically reprensets this object and adds it to Layer
    virtual void initializeGraphics(Layer* layer) = 0;
    
    Vec2 getInitialCenterPix();

    util::multifunction<void()> multiInit;
    util::multifunction<void()> multiUpdate;
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
    virtual inline shared_ptr<cp::Body> initializeBody(cp::Space& space)
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
    virtual inline shared_ptr<cp::Body> initializeBody(cp::Space& space)
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

    void update();
    Node* sprite;
};

//Initialize graphics from a still image. Any class that uses this mixin has to implement interface to
//provide the path to the image file.
class ImageSprite : public virtual SpriteObject
{
public:
    virtual string imageSpritePath() const = 0;
    virtual GraphicsLayer sceneLayer() const = 0;
    void loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest);
    inline void initializeGraphics(Layer* layer)
    {
        loadImageSprite(imageSpritePath(), GraphicsLayer::ground, layer);
    }
};

class PatchConSprite : virtual public SpriteObject, RegisterUpdate<PatchConSprite>
{
public:
    inline PatchConSprite() : RegisterUpdate<PatchConSprite>(this ) {}
    virtual string imageSpritePath() const = 0;
    virtual GraphicsLayer sceneLayer() const = 0;
    
    inline void initializeGraphics(Layer* layer)
    {
        animSprite = PatchConAnimation::create();
        animSprite->loadAnimation(imageSpritePath());
        layer->positionAndAddNode(animSprite, sceneLayer(), getInitialCenterPix());
        sprite = animSprite;
    }
    
    inline void update()
    {
        SpaceVect dist = body->getVel()*App::secondsPerFrame;
        
        animSprite->accumulate(dist.length());
    }
protected:
    PatchConAnimation* animSprite;
};

#endif /* GObject_hpp */
