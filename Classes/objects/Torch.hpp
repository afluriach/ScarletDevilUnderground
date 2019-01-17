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

class Torch :
	public virtual GObject,
	public RectangleBody,
	public InteractibleObject,
	public RegisterUpdate<Torch>
{
public:
	static const unordered_map<string, Color3B> colorMap;

	Torch(GSpace* space, ObjectIDType id, const ValueMap& args);

    virtual inline SpaceFloat getMass() const {return -1.0;}
    virtual inline GType getType() const {return GType::environment;}
    virtual inline SpaceVect getDimensions() const {return SpaceVect(1,1);}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual inline bool canInteract() { return true; }
	virtual void interact();
	virtual inline string interactionIcon() { return "sprites/blue_torch.png"; }

    virtual void initializeGraphics();
    
    void setActive(bool active);
    bool getActive();
    
    void update();    
protected:
	string colorName;
	unsigned int lightSourceID = 0;
	unsigned int baseSpriteID = 0;
	unsigned int flameSpriteID = 0;
	Color3B color = Color3B(255,255,255);
	float intensity = 1.0f;
	//The ratio of radius where the light is at full intensity.
	float flood = 0.5f;
	float lightRadius = 3.0f;
	bool isActive = false;
};

#endif /* Torch_hpp */
