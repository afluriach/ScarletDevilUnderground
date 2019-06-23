//
//  Rumia.hpp
//  Koumachika
//
//  Created by Toni on 2/28/19.
//
//

#ifndef Rumia_hpp
#define Rumia_hpp

#include "Enemy.hpp"

class Rumia : public Enemy
{
public:
	MapObjCons(Rumia);

    virtual inline SpaceFloat getMass() const {return 20.0;}

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }
    
	virtual shared_ptr<LightArea> getLightSource() const;
    virtual inline string getSprite() const {return "rumia";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
};

class Rumia1 : public Rumia, public BaseAttributes<Rumia1>
{
public:
	static const string baseAttributes;
	static const string properName;

	MapObjCons(Rumia1);

	inline virtual string initStateMachine() { return "rumia1"; }
	virtual void onZeroHP();
};

class Rumia2 : public Rumia, public BaseAttributes<Rumia2>
{
public:
	static const string baseAttributes;
	static const string properName;

	MapObjCons(Rumia2);

	inline virtual string initStateMachine() { return "rumia2"; }
	virtual void onZeroHP();
};

#endif /* Rumia_hpp */
