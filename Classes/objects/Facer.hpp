//
//  Facer.hpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#ifndef Facer_hpp
#define Facer_hpp

#include "Enemy.hpp"

class Facer : public Enemy, public BaseAttributes<Facer>
{
public:
	static const string baseAttributes;
	static const string properName;

	MapObjCons(Facer);

    inline SpaceFloat getMass() const {return 40.0;}

    inline string getSprite() const {return "facer";}
 
	inline virtual string initStateMachine() { return "facer"; }
};

#endif /* Facer_hpp */
