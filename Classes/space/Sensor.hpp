//
//  Sensor.hpp
//  Koumachika
//
//  Created by Toni on 2/6/20.
//
//

#ifndef Sensor_hpp
#define Sesnor_hpp

class Sensor
{
public:
	friend class GSpace;

	inline Sensor() {}
	inline virtual ~Sensor() {}

	virtual void collision(GObject* other) = 0;
	virtual void endCollision(GObject* other) = 0;
};

#endif
