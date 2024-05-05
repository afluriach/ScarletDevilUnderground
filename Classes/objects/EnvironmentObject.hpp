//
//  EnvironmentObject.hpp
//  Koumachika
//
//  Created by Toni on 2/5/20.
//
//

#ifndef EnvironmentObject_hpp
#define EnvironmentObject_hpp

class EnvironmentObject : public GObject
{
public:
	EnvironmentObject(
		GSpace* space,
		ObjectIDType id,
		const object_params& params,
		local_shared_ptr<environment_object_properties> props
	);

	~EnvironmentObject();

	virtual bool hit(DamageInfo damage, SpaceVect n);

	virtual void init();
	virtual void update();

	virtual string interactionIcon(Player* p);
	virtual bool canInteract(Player* p);
	virtual void interact(Player* p);

	virtual shared_ptr<sprite_properties> getSprite() const;
	virtual GraphicsLayer sceneLayer() const;
protected:
	local_shared_ptr<environment_object_properties> props;
};

#endif
