//
//  Wall.hpp
//  Koumachika
//
//  Created by Toni on 4/6/18.
//
//

#ifndef Wall_hpp
#define Wall_hpp

#include "GObject.hpp"

class Wall : public GObject
{
public: 
	MapObjCons(Wall);
	Wall(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions);
	virtual inline ~Wall() {}

    virtual inline GType getType() const {return GType::wall;}
    virtual PhysicsLayers getLayers() const;    
};

class BreakableWall : public Wall
{
public:
	MapObjCons(BreakableWall);
	void hit();
	void applyBreak();
};


#endif /* Wall_hpp */
