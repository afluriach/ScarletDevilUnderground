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

class EffectArea : public AreaSensor 
{
public:
	EffectArea(
		GSpace* space,
		ObjectIDType id,
		const object_params& params,
		local_shared_ptr<effectarea_properties> props
	);
	inline virtual ~EffectArea(){}
    
	virtual void beginContact(GObject* obj);
	virtual void endContact(GObject* obj);
protected:
	local_shared_ptr<effectarea_properties> props;
	
	map<GObject*, unsigned int> activeEffects;
	effect_attributes attr;
};

#endif /* EffectArea_hpp */
