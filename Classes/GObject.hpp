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
    typedef std::function<GObject*( const cocos2d::ValueMap&) > AdapterType;
    
    //Representation as a map object
    GObject(const cocos2d::ValueMap& args);
    GObject(const string& name, const cp::Vect& pos);
    
    virtual ~GObject() {}
    
    //Map each class name to a constructor adapter function.
    static const std::map<std::string,AdapterType> adapters;
    
    static GObject* constructByType(const std::string& type, const cocos2d::ValueMap& args );
    
    static ValueMap makeValueMapArg(const Vec2& pos, const map<string,string>& props);

    const std::string name;
    
    std::shared_ptr<cp::Body> body;
    
    //Posiition where the object was loaded
    cp::Vect initialCenter;
    
    inline void addInit(std::function<void()> f){
        initializers.push_back(f);
    }
    
    //Called on the first frame after it has been added, before update is called on it or any other
    //objects in the same frame
    inline void init()
    {
        BOOST_FOREACH(std::function<void()> f, initializers){
            f();
        }
    }
    
    inline void addUpdater(std::function<void()> f){
        updaters.push_back(f);
    }
    
    inline void update()
    {
        BOOST_FOREACH(std::function<void()> f, updaters){
            f();
        }
    }
    
    //Called before adding the the object to space.
    virtual std::shared_ptr<cp::Body>initializeBody(cp::Space& space) = 0;
    
    //Create Node which graphically reprensets this object and adds it to Layer
    virtual void initializeGraphics(cocos2d::Layer* layer) = 0;
    
    cocos2d::Vec2 getInitialCenterPix();
private:
    std::vector<std::function<void()>> initializers;
    std::vector<std::function<void()>> updaters;
};

template<typename Derived>
class RegisterInit : public virtual GObject
{
public:
    inline RegisterInit(Derived* that)
    {
        addInit(std::bind(&Derived::init, that));
    }
};

template<typename Derived>
class RegisterUpdate : public virtual GObject
{
public:
    inline RegisterUpdate(Derived* that)
    {
        addUpdater(std::bind(&Derived::update, that));
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
    virtual inline std::shared_ptr<cp::Body> initializeBody(cp::Space& space)
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
    
    virtual inline cp::Vect getDimensions() const = 0;
};

class RectangleMapBody : public virtual RectangleBody
{
public:
    static cp::Vect getDimensionsFromMap(const ValueMap& arg);

    inline RectangleMapBody(const ValueMap& arg) : dim(getDimensionsFromMap(arg)) {}
    
    inline cp::Vect getDimensions() const { return dim;}

private:
    //Rectular dimensions or BB dimensions if object is not actually rectangular.
    cp::Vect dim;
};

class CircleBody : public virtual PhysicsObject
{
public:
    virtual float getRadius() const = 0;

    //Create body and add it to space. This assumes BB is rectangle dimensions
    virtual inline std::shared_ptr<cp::Body> initializeBody(cp::Space& space)
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
    cocos2d::Node* sprite;
};

//Initialize graphics from a still image. Any class that uses this mixin has to implement interface to
//provide the path to the image file.
class ImageSprite : public virtual SpriteObject
{
public:
    virtual string imageSpritePath() const = 0;
    virtual GraphicsLayer sceneLayer() const = 0;
    void loadImageSprite(const std::string& resPath, GraphicsLayer sceneLayer, cocos2d::Layer* dest);
    inline void initializeGraphics(cocos2d::Layer* layer)
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
    
    inline void initializeGraphics(cocos2d::Layer* layer)
    {
        animSprite = PatchConAnimation::create();
        animSprite->loadAnimation(imageSpritePath());
        layer->positionAndAddNode(animSprite, sceneLayer(), getInitialCenterPix());
        sprite = animSprite;
    }
    
    inline void update()
    {
        cp::Vect dist = body->getVel()*App::secondsPerFrame;
        
        animSprite->accumulate(dist.length());
    }
protected:
    PatchConAnimation* animSprite;
};

#endif /* GObject_hpp */
