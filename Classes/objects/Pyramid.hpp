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

class Pyramid :
virtual public GObject,
public RadarObject
{
public:
	static const SpaceFloat coneLength;
	static const SpaceFloat coneWidth;
	static const unsigned int coneSegments;
	static const Color4F coneColor;
	static const Color4F coneActiveColor;

	static const DamageInfo lightConeEffect;

	MapObjCons(Pyramid);
    
	virtual void update();
	void redrawLightCone();

    virtual string getSprite() const {return "pyramid";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::environment;}
    virtual PhysicsLayers getLayers() const;

	virtual inline SpaceFloat getRadarRadius() const { return coneLength; }
	virtual inline GType getRadarType() const { return GType::enemySensor; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return coneWidth/2.0; }

	virtual void onDetect(GObject* other);
	virtual void onEndDetect(GObject* other);

	virtual shared_ptr<LightArea> getLightSource() const;
protected:
	unordered_set<object_ref<Agent>> targets;
	SpaceFloat angular_speed = 0.0;
	SpaceFloat coneAngle = 0.0;
};
#endif /* Pyramid_hpp */
