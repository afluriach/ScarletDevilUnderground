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

class EffectArea :
public AreaSensor, 
public RegisterUpdate<EffectArea>
{
public:
	MapObjCons(EffectArea);
	inline virtual ~EffectArea(){}
    
	void update();

	virtual inline AttributeMap getAttributeEffect() { return {}; }
	virtual inline shared_ptr<MagicEffect> getMagicEffect(GObject* target) { return nullptr; }
};

class SunArea : public EffectArea
{
public:
	MapObjCons(SunArea);

	virtual void initializeGraphics();
	virtual GraphicsLayer sceneLayer() const;
	virtual AttributeMap getAttributeEffect();
};

class DarknessArea :
	public EffectArea,
	public RegisterInit<DarknessArea>,
	public RegisterUpdate<DarknessArea>
{
public:
	MapObjCons(DarknessArea);

	void init();
	void update();

	virtual AttributeMap getAttributeEffect();
protected:
	set<Torch*> torches;
	bool active = false;
};

#endif /* EffectArea_hpp */
