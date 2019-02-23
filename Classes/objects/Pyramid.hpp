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
public RegisterUpdate<Pyramid>,
public RadarObject,
public RectangleBody
{
public:
	static const SpaceFloat coneLength;
	static const SpaceFloat coneWidth;
	static const unsigned int coneSegments;
	static const Color4F coneColor;
	static const Color4F coneActiveColor;

	static const AttributeMap lightConeEffect;

	MapObjCons(Pyramid);
    
	void update();
	void redrawLightCone();

    virtual string imageSpritePath() const {return "sprites/pyramid_base.png";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline SpaceFloat getMass() const { return -1.0;}
    virtual inline GType getType() const {return GType::environment;}
    virtual PhysicsLayers getLayers() const;

	virtual inline SpaceFloat getRadarRadius() const { return coneLength; }
	virtual inline GType getRadarType() const { return GType::enemySensor; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return coneWidth/2.0; }

	virtual void onDetect(GObject* other);
	virtual void onEndDetect(GObject* other);

	virtual void initializeGraphics();
protected:
	set<object_ref<Agent>> targets;
	SpaceFloat angular_speed = 0.0;
	SpaceFloat coneAngle = 0.0;
	LightID coneLightID;
};
#endif /* Pyramid_hpp */
