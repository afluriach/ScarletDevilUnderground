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

#include "chipmunk.hpp"
#include "cocos2d.h"

#include "util.h"

class GObject
{
public:
    typedef std::function<GObject*( const cocos2d::ValueMap&) > AdapterType;
    
    //Representation as a map object
    GObject(const cocos2d::ValueMap& args);
    //Map each class name to a constructor adapter function.
    static const std::map<std::string,AdapterType> adapters;
    
    static GObject* constructByType(const std::string& type, const cocos2d::ValueMap& args );

    const std::string name;
    
    std::shared_ptr<cp::Body> body;
    //A default of 0 signifies undefined. Using -1 to indicate static.
    float mass = 0;
    
    //Posiition where the object was loaded
    cp::Vect initialCenter;
    //Rectular dimensions or BB dimensions if object is not actually rectangular.
    cp::Vect dim;
    
    virtual std::shared_ptr<cp::Body>initializeBody(cp::Space& space) = 0;
    
    //Create body and add it to space. This assumes BB is rectangle dimensions
    std::shared_ptr<cp::Body> initRectangleBody(cp::Space& space);
    //Create Node which graphically reprensets this object and adds it to Layer
    virtual void initializeGraphics(cocos2d::Layer* layer) = 0;
};

#endif /* GObject_hpp */
