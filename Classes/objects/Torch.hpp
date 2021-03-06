//
//  Torch.hpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#ifndef Torch_hpp
#define Torch_hpp

class Torch : public GObject
{
public:
	static const unordered_map<string, Color3B> colorMap;
	static const float darknessDrain;

	Torch(GSpace* space, ObjectIDType id, const ValueMap& args);

    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual inline bool canInteract(Player* p) { return true; }
	virtual void interact(Player* p);
	virtual inline string interactionIcon(Player* p) { return "sprites/torch.png"; }

    virtual void initializeGraphics();
	virtual sprite_update updateSprite();

    void setActive(bool active);
    bool getActive();    
protected:
	void addLightSource();

	string colorName;
	SpriteID flameSpriteID = 0;
	Color3B color = Color3B(255,255,255);
	float intensity = 1.0f;
	//The ratio of radius where the light is at full intensity.
	float flood = 0.5f;
	float lightRadius = 3.0f;
	bool isActive = false;
};

#endif /* Torch_hpp */
