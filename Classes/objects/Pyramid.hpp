//
//  Pyramid.hpp
//  Koumachika
//
//  Created by Toni on 12/14/18.
//
//

#ifndef Pyramid_hpp
#define Pyramid_hpp

#include "AIMixins.hpp"
#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Pyramid :
virtual public GObject,
public StateMachineObject,
public RegisterInit<Pyramid>,
public RegisterUpdate<Pyramid>,
public RadarObject,
public RectangleMapBody
{
public:
	static const SpaceFloat coneLength;
	static const SpaceFloat coneAngle;
	static const unsigned int coneSegments;
	static const Color4F coneColor;
	static const Color4F coneActiveColor;

	static const AttributeMap lightConeEffect;

	MapObjCons(Pyramid);
    
	void init();
	void update();
	void redrawLightCone();

    virtual string imageSpritePath() const {return "sprites/pyramid_base.png";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline SpaceFloat getMass() const { return -1.0;}
    virtual inline GType getType() const {return GType::environment;}
    virtual PhysicsLayers getLayers() const;
    virtual inline SpaceVect getDimensions() const {return SpaceVect(1,1);}

	virtual inline SpaceFloat getRadarRadius() const { return coneLength; }
	virtual inline GType getRadarType() const { return GType::playerSensor; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return coneAngle/2.0; }

	virtual void onDetect(GObject* other);
	virtual void onEndDetect(GObject* other);

	virtual void initializeGraphics();

	void initStateMachine(ai::StateMachine& sm);
protected:
	set<object_ref<Agent>> targets;
	SpaceFloat angular_speed = 0.0;
	boost::rational<int> discrete_look = 0;
};
#endif /* Pyramid_hpp */
