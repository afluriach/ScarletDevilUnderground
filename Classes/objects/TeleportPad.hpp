//
//  TeleportPad.hpp
//  Koumachika
//
//  Created by Toni on 12/29/18.
//
//

#ifndef TeleportPad_hpp
#define TeleportPad_hpp

#include "AreaSensor.hpp"

class Agent;

class TeleportPad : public AreaSensor
{
public:
	MapObjCons(TeleportPad);
    
	virtual bool isObstructed() const;
	void setTeleportActive(bool v);
protected:
	bool teleportActive = false;
};

#endif /* EffectArea_hpp */
