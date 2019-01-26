//
//  Block.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef Block_hpp
#define Block_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Block :
	virtual public GObject,
	public RectangleBody,
	public ImageSprite,
	public FrictionObject,
	public MassImpl
{
public:
	MapObjCons(Block);
    
    virtual string imageSpritePath() const {return "sprites/block.png";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline SpaceFloat uk() const {return 0.5;}
    virtual inline GType getType() const {return GType::environment;}
    virtual PhysicsLayers getLayers() const;
};
#endif /* Block_hpp */
