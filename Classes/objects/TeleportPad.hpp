//
//  TeleportPad.hpp
//  Koumachika
//
//  Created by Toni on 12/29/18.
//
//

#ifndef TeleportPad_hpp
#define TeleportPad_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "object_ref.hpp"

class Agent;

class TeleportPad :
virtual public GObject,
public RectangleMapBody,
public NoSprite
{
public:
	MapObjCons(TeleportPad);
    
    virtual inline SpaceFloat getMass() const { return -1.0;}
	virtual inline bool getSensor() const { return true; }
    virtual inline GType getType() const {return GType::teleportPad;}
    virtual PhysicsLayers getLayers() const;

	void onContact(GObject* obj);
	void onEndContact(GObject* obj);

	bool isObstructed() const;
	void setTeleportActive(bool v);
protected:
	set<gobject_ref> contacts;
	bool teleportActive = false;
};

#endif /* EffectArea_hpp */
