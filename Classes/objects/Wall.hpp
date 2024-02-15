//
//  Wall.hpp
//  Koumachika
//
//  Created by Toni on 4/6/18.
//
//

#ifndef Wall_hpp
#define Wall_hpp

class Wall : public GObject
{
public: 
	static GType getWallType(local_shared_ptr<wall_properties> props);

	Wall(
		GSpace* space,
		ObjectIDType id,
		const object_params& params,
		local_shared_ptr<wall_properties> props
	);

	virtual inline ~Wall() {}

	virtual bool hit(DamageInfo damage, SpaceVect n);
	void applyBreak();
	
	DamageInfo getTouchDamage() const { return props ? props->touchDamage : DamageInfo(); }
protected:
	local_shared_ptr<wall_properties> props;
};

#endif /* Wall_hpp */
