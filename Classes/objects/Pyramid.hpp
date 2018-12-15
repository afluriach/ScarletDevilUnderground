//
//  Pyramid.hpp
//  Koumachika
//
//  Created by Toni on 12/14/18.
//
//

#ifndef Pyramid_hpp
#define Pyramid_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Pyramid :
virtual public GObject,
public RectangleBody
{
public:
	MapObjCons(Pyramid);
    
    virtual string imageSpritePath() const {return "sprites/pyramid_base.png";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline SpaceFloat getMass() const { return -1.0;}
    virtual inline GType getType() const {return GType::environment;}
    virtual PhysicsLayers getLayers() const;
    virtual inline SpaceVect getDimensions() const {return SpaceVect(1,1);}

	virtual void initializeGraphics(SpaceLayer* layer);
};
#endif /* Pyramid_hpp */
