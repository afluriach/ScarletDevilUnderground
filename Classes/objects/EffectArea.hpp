//
//  EffectArea.hpp
//  Koumachika
//
//  Created by Toni on 12/14/18.
//
//

#ifndef EffectArea_hpp
#define EffectArea_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "object_ref.hpp"

class Agent;

class EffectArea :
virtual public GObject,
public RectangleMapBody,
public RegisterUpdate<EffectArea>
{
public:
	MapObjCons(EffectArea);
	inline virtual ~EffectArea(){}
    
    virtual inline SpaceFloat getMass() const { return -1.0;}
	virtual inline bool getSensor() const { return true; }
    virtual inline GType getType() const {return GType::effectArea;}
    virtual PhysicsLayers getLayers() const;

	void update();

	void onContact(Agent* agent);
	void onEndContact(Agent* agent);

	virtual inline AttributeMap getAttributeEffect() { return {}; }
	virtual inline shared_ptr<MagicEffect> getMagicEffect(GObject* target) { return nullptr; }
protected:
	set<object_ref<Agent>> contacts;
};

class SunArea : public EffectArea
{
public:
	MapObjCons(SunArea);

	virtual void initializeGraphics();
	virtual GraphicsLayer sceneLayer() const;
	virtual AttributeMap getAttributeEffect();
};

#endif /* EffectArea_hpp */
