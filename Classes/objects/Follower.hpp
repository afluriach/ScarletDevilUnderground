//
//  Follower.hpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#ifndef Follower_hpp
#define Follower_hpp

#include "Enemy.hpp"

class Follower : public Enemy, public BaseAttributes<Follower>
{
public:
	static const string baseAttributes;

	MapObjCons(Follower);

    inline SpaceFloat getMass() const {return 40.0;}

    inline string getSprite() const {return "follower";}        
	virtual inline string initStateMachine() { return "follower"; }
};


#endif /* Follower_hpp */
