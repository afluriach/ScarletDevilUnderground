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
    
	virtual void beginContact(GObject* obj);
	virtual void endContact(GObject* obj);

	virtual void update();

	virtual inline DamageInfo getDamageInfo() const { return DamageInfo(); }
protected:
	unordered_set<GObject*> targets;
};

class SunArea : public EffectArea
{
public:
	MapObjCons(SunArea);

	virtual shared_ptr<LightArea> getLightSource() const;
	virtual GraphicsLayer sceneLayer() const;
	virtual DamageInfo getDamageInfo() const;
};

class DarknessArea : public EffectArea
{
public:
	MapObjCons(DarknessArea);

	virtual void init();
	virtual void update();

	virtual DamageInfo getDamageInfo() const;
protected:
	unordered_set<Torch*> torches;
	bool active = false;
};

#endif /* EffectArea_hpp */
