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

class FirePattern;

class Launcher :
virtual public GObject,
public RectangleBody,
public ActivateableObject
{
public:
	static const boost::rational<int> fireInterval;

	Launcher(GSpace* space, ObjectIDType id, const ValueMap& args);

    virtual inline SpaceFloat getMass() const {return -1.0;}
    virtual inline GType getType() const {return GType::environment;}

    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
	virtual inline string getSprite() const { return "launcher"; }

	virtual inline void activate() { isActive = true; }
	virtual inline void deactivate() { isActive = false; }
    
    virtual void update();    
protected:
    bool isActive = false;
	boost::rational<int> cooldownTime = 0;
};

#endif /* Launcher_hpp */
