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
	EnvironmentObject(GSpace* space, ObjectIDType id, const ValueMap& args, local_shared_ptr<object_properties> props);

	virtual bool hit(DamageInfo damage, SpaceVect n);

	virtual string getSprite() const;
	virtual GraphicsLayer sceneLayer() const;

	virtual SpaceFloat uk() const;
protected:
	local_shared_ptr<object_properties> props;
};

#endif
