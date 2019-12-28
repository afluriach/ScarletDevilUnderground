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
	static GType getWallType(bool breakable);

	MapObjCons(Wall);
	Wall(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions, bool breakable = false);

	virtual inline ~Wall() {}

	virtual bool hit(DamageInfo damage, SpaceVect n);
	void applyBreak();
protected:
	bool breakable;
};

#endif /* Wall_hpp */
