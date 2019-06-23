//
//  Sakuya.hpp
//  Koumachika
//
//  Created by Toni on 4/17/18.
//
//

#ifndef Sakuya_hpp
#define Sakuya_hpp

#include "Enemy.hpp"

class Sakuya :
public Enemy,
public BaseAttributes<Sakuya>
{
public:
	static const string baseAttributes;

	MapObjCons(Sakuya);

    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    virtual inline SpaceFloat getDefaultFovAngle() const {return 0.0;}

    inline SpaceFloat getMass() const {return 40.0;}

    inline string getSprite() const {return "sakuya";}

	virtual inline string initStateMachine() { return "sakuya"; }
};

#endif /* Sakuya_hpp */
