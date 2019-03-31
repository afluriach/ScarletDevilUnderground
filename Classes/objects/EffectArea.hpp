//
//  EffectArea.hpp
//  Koumachika
//
//  Created by Toni on 12/14/18.
//
//

#ifndef EffectArea_hpp
#define EffectArea_hpp

#include "AreaSensor.hpp"

class Agent;
class Torch;

class EffectArea : public AreaSensor 
{
public:
	MapObjCons(EffectArea);
	inline virtual ~EffectArea(){}
    
	virtual void update();

	virtual inline DamageInfo getDamageInfo() const { return DamageInfo{}; }
};

class SunArea : public EffectArea
{
public:
	MapObjCons(SunArea);

	virtual void initializeGraphics();
	virtual GraphicsLayer sceneLayer() const;
	virtual DamageInfo getDamageInfo() const;
};

class DarknessArea : public EffectArea, public RegisterInit<DarknessArea>
{
public:
	MapObjCons(DarknessArea);

	void init();
	virtual void update();

	virtual DamageInfo getDamageInfo() const;
protected:
	unordered_set<Torch*> torches;
	bool active = false;
};

#endif /* EffectArea_hpp */
