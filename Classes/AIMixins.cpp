//
//  AIMixins.cpp
//  Koumachika
//
//  Created by Toni on 3/14/18.
//
//

#include "Prefix.h"
#include "AIMixins.hpp"

const float ObjectSensor::coneHalfWidth = float_pi / 4;


GObject* ObjectSensor::getSensedObject()
{
	SpaceVect facingUnit = SpaceVect::ray(1, body->getAngle());
	//The dot of the object's facing unit vector with the unit vector representing
	//the displacement to the object should be at least this much to be considered.
	float scalarMinimum = cos(ObjectSensor::coneHalfWidth);

	float bestScalar = -1;
	GObject* bestObj = nullptr;

	BOOST_FOREACH(GObject* obj, inRange)
	{
		SpaceVect displacementUnit = (obj->getPos() - getPos()).normalize();

		float dot = SpaceVect::dot(facingUnit, displacementUnit);

		if (dot > bestScalar && dot >= scalarMinimum)
		{
			bestScalar = dot;
			bestObj = obj;
		}
	}

	return bestObj;
}
