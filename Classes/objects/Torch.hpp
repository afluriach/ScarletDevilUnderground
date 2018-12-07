//
//  Torch.hpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#ifndef Torch_hpp
#define Torch_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Torch : public virtual GObject, RectangleBody, RegisterUpdate<Torch>
{
public:
    inline Torch(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	RegisterUpdate<Torch>(this)
    {}

    virtual inline float getMass() const {return -1;}
    virtual inline GType getType() const {return GType::environment;}
    virtual inline SpaceVect getDimensions() const {return SpaceVect(1,1);}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

    virtual void initializeGraphics(Layer* layer);
    
    void setActive(bool active);
    bool getActive();
    
    void update();
    
    TimedLoopAnimation* flame;
    
protected:
    bool isActive;
};

#endif /* Torch_hpp */
