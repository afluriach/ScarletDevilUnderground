//
//  EnvironmentObject.hpp
//  Koumachika
//
//  Created by Toni on 2/5/20.
//
//

#ifndef EnvironmentObject_hpp
#define EnvironmentObject_hpp

class environment_object_properties : public object_properties
{
public:
	string scriptName;
	string interactionIcon;
	PhysicsLayers layers = PhysicsLayers::none;
	bool interactible;
};

class EnvironmentObject : public GObject
{
public:
	EnvironmentObject(
		GSpace* space,
		ObjectIDType id,
		const ValueMap& args,
		local_shared_ptr<environment_object_properties> props
	);

	virtual bool hit(DamageInfo damage, SpaceVect n);

	virtual void init();
	virtual void update();

	virtual string interactionIcon(Player* p);
	virtual bool canInteract(Player* p);
	virtual void interact(Player* p);

	virtual string getSprite() const;
	virtual GraphicsLayer sceneLayer() const;

	virtual SpaceFloat uk() const;
protected:
	local_shared_ptr<environment_object_properties> props;
};

#endif
