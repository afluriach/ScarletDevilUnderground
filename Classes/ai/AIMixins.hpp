//
//  AIMixins.hpp
//  Koumachika
//
//  Created by Toni on 3/14/18.
//
//

#ifndef AIMixins_hpp
#define AIMixins_hpp

#include "AI.hpp"
#include "GObject.hpp"
#include "GObjectMixins.hpp"

class RadarSensor;

class RadarObject : virtual public GObject
{
public:
	RadarObject();

	void _update();

	virtual SpaceFloat getRadarRadius() const = 0;
	virtual GType getRadarType() const = 0;
    virtual inline SpaceFloat getDefaultFovAngle() const {return 0.0;}
	virtual inline bool hasEssenceRadar() const { return false; }

	virtual void onDetect(GObject* other) = 0;
	virtual void onEndDetect(GObject* other) = 0;
    
	//Create body and add it to space. This assumes BB is rectangle dimensions
	virtual void initializeRadar(GSpace& space);
	virtual void removePhysicsObjects();

	inline RadarSensor* getRadar() { return radar; }
protected:
	RadarSensor* radar = nullptr;
};

#endif /* AIMixins_hpp */
