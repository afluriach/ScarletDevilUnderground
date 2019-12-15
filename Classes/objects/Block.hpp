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

class Block : public GObject
{
public:
	MapObjCons(Block);
    
    virtual string getSprite() const {return "block";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline SpaceFloat uk() const {return 0.5;}
    virtual PhysicsLayers getLayers() const;
};
#endif /* Block_hpp */
