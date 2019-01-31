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
#include "GObjectMixins.hpp"

class Wall : virtual public GObject, public RectangleBody, public NoSprite
{
public: 
	MapObjCons(Wall);
	Wall(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions);
    
    virtual inline SpaceFloat getMass() const { return -1.0;}
    virtual inline GType getType() const {return GType::wall;}
    virtual PhysicsLayers getLayers() const;    
};

class BreakableWall : public Wall
{
public:
	static const int defaultHP;

	MapObjCons(BreakableWall);
	void onCollide(Bullet* b);
protected:
	int hp;
};


#endif /* Wall_hpp */
