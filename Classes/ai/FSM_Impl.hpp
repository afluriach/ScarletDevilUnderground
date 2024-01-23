//
//  FSM_Impl.hpp
//  Koumachika
//
//  Created by Toni on 1/23/24.
//

#ifndef FSM_Impl_hpp
#define FSM_Impl_hpp

#include "AI.hpp"

namespace ai{

class FSM_Impl : public FSM
{
public:
    inline virtual ~FSM_Impl() {}
    
    virtual void update();
    
    virtual void onDetectEnemy(Agent* enemy);
	virtual void onEndDetectEnemy(Agent* enemy);
	virtual void onDetectBomb(Bomb* bomb);
	virtual void onDetectBullet(Bullet* bullet);
 
 	virtual void onBulletHit(Bullet* b);
	virtual void onBulletBlock(Bullet* b);
	virtual void enemyRoomAlert(Agent* enemy);
	virtual void onZeroHP();
	virtual void onZeroStamina();
 
    virtual string toString();
};

}

#endif
