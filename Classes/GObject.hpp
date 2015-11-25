//
//  GObject.hpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GObject_hpp
#define GObject_hpp

#include <map>
#include <memory>
#include <vector>

#include <boost/foreach.hpp>

#include "chipmunk.hpp"
#include "cocos2d.h"

#include "GAnimation.hpp"
#include "PlayScene.hpp"
#include "util.h"

class GObject
{
public:
    typedef std::function<GObject*( const cocos2d::ValueMap&) > AdapterType;
    
    //Representation as a map object
    GObject(const cocos2d::ValueMap& args);
    GObject(const string& name, const cp::Vect& pos, const cp::Vect& dim);
    
    virtual ~GObject() {}
    
    //Map each class name to a constructor adapter function.
    static const std::map<std::string,AdapterType> adapters;
    
    static GObject* constructByType(const std::string& type, const cocos2d::ValueMap& args );
    
    static ValueMap makeValueMapArg(const Vec2& pos, const Vec2& dim, const map<string,string>& props);

    const std::string name;
    
    std::shared_ptr<cp::Body> body;
    
    //Posiition where the object was loaded
    cp::Vect initialCenter;
    //Rectular dimensions or BB dimensions if object is not actually rectangular.
    cp::Vect dim;
    
    //Called on the first frame after it has been added, before update is called on it or any other
    //objects in the same frame
    virtual void init() = 0;
    
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
    std::vector<std::function<void()>> updaters;
};

class PhysicsObject : public virtual GObject
{
public:
    //A default of 0 signifies undefined. Using -1 to indicate static or positive for dynamic.
    virtual float getMass() const = 0;
    virtual GSpace::Type getType() const = 0;
    virtual inline bool getSensor() const {return false;}
    virtual inline int getLayers() const {return GSpace::Layers::ground;}
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
            dim,
            getMass(),
            getType(),
            getLayers(),
            getSensor(),
            this
        );
        return body;
    }
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

class SpriteObject : public virtual GObject
{
public:
    SpriteObject()
    {
        addUpdater(std::bind(&SpriteObject::updateSpritePos, this));
    }
    
    void updateSpritePos();
    cocos2d::Node* sprite;
};

//Initialize graphics from a still image. Any class that uses this mixin has to implement interface to
//provide the path to the image file.
class ImageSprite : public virtual SpriteObject
{
public:
    virtual string imageSpritePath() const = 0;
    virtual GScene::Layer sceneLayer() const = 0;
    void loadImageSprite(const std::string& resPath, GScene::Layer sceneLayer, cocos2d::Layer* dest);
    inline void initializeGraphics(cocos2d::Layer* layer)
    {
        loadImageSprite(imageSpritePath(), GScene::Layer::ground, layer);
    }
};

class PatchConSprite : virtual public SpriteObject
{
public:
    virtual string imageSpritePath() const = 0;
    virtual GScene::Layer sceneLayer() const = 0;
    
    inline void initializeGraphics(cocos2d::Layer* layer)
    {
        animSprite = PatchConAnimation::create();
        animSprite->loadAnimation(imageSpritePath());
        positionAndAddNode(animSprite, sceneLayer(), layer, getInitialCenterPix());
        sprite = animSprite;
    }
    
    inline PatchConSprite()
    {
        addUpdater(std::bind(&PatchConSprite::updateAnimation, this));
    }
    
    inline void updateAnimation()
    {
        cp::Vect dist = body->getVel()*App::secondsPerFrame;
        
        animSprite->accumulate(dist.length());
    }
protected:
    PatchConAnimation* animSprite;
};

#endif /* GObject_hpp */
