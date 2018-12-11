//
//  Launcher.hpp
//  Koumachika
//
//  Created by Toni on 12/11/18.
//
//

#ifndef Launcher_hpp
#define Launcher_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "types.h"

class FirePattern;

class Launcher :
virtual public GObject,
public RectangleBody,
public ActivateableObject,
public ImageSprite,
public RegisterUpdate<Launcher>
{
public:
	static const int fireInterval;

	Launcher(GSpace* space, ObjectIDType id, const ValueMap& args);

    virtual inline float getMass() const {return -1;}
    virtual inline GType getType() const {return GType::environment;}
    virtual inline SpaceVect getDimensions() const {return SpaceVect(1,1);}

    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
	virtual inline string imageSpritePath() const { return "sprites/launcher.png"; }

	virtual inline void activate() { isActive = true; }
	virtual inline void deactivate() { isActive = false; }
    
    void update();    
protected:
    bool isActive = false;
	int cooldownFrames = 0;
};

#endif /* Launcher_hpp */
