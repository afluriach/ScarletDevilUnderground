//
//  types.h
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef types_h
#define types_h

#include "vect.hpp"

namespace ai { class StateMachine; }
class Bullet;
class GObject;
class GSpace;
class Player;

typedef pair<float, float> float_pair;
typedef pair<int,int> IntVec2;
typedef tuple<long, long, long> TimerTriplet;
typedef map<type_index, pair<unsigned int, unsigned int>> EnemyStatsMap;
typedef cpFloat SpaceFloat;
typedef vector<SpaceVect> Path;

typedef unsigned int LightID;
typedef unsigned int SpriteID;
typedef unsigned int ObjectIDType;

typedef function<void(void)> zero_arity_function;

typedef function<GObject*(GSpace*, ObjectIDType)> ObjectGeneratorType;
typedef function<FiniteTimeAction*()> ActionGeneratorType;

typedef function<void(ai::StateMachine&, Player*)> alert_function;
typedef function<void(ai::StateMachine&, GObject*)> detect_function;
typedef function<void(ai::StateMachine&, Bullet*)> bullet_collide_function;

namespace boost {
	namespace serialization {
		template<class Archive>
		inline void serialize(Archive & ar, SpaceVect & v, const unsigned int version)
		{
			ar & v.x;
			ar & v.y;
		}
	}
}

class SpaceRect
{
public:
	SpaceRect();
	SpaceRect(SpaceVect center, SpaceVect dimensions);
	SpaceRect(SpaceFloat llX, SpaceFloat llY, SpaceFloat width, SpaceFloat height);

	CCRect toPixelspace() const;

	double getMinX() const;
	double getMinY() const;
	double getMaxX() const;
	double getMaxY() const;

	SpaceVect getLLCorner() const;
	SpaceVect getURCorner() const;

	bool intersectsRect(const SpaceRect& rect) const;
	bool containsPoint(const SpaceVect& point) const;

	SpaceVect center, dimensions;
};

enum class ChamberID
{
	begin = 0,
	graveyard0 = begin,
	graveyard1,
	graveyard2,
	forest1,
	desert1,
	mine1,

	end,
	invalid_id = end
};

bool isValidChamber(ChamberID id);

enum class Direction{
    none = 0,
    right,
    up,
    left,
    down,
    end,
};

enum class GraphicsLayer{
	begin = 1,
    map = 1,
    foliage,
    floor,
    ground,
	roomMask,
	agentOverlay,
	overhead,

	end
};

enum class GType{
	none = 0x0,
	
	player = 0x1,
    playerBullet = 0x2,
    enemy = 0x4,
    enemyBullet = 0x8,
    environment = 0x10,
    foliage = 0x20,
    wall = 0x40,
	areaSensor = 0x80,
    enemySensor = 0x100,
    playerGrazeRadar = 0x200,
    collectible = 0x400,
	upgrade = 0x800,
    npc = 0x1000,
	floorSegment = 0x2000,
	bomb = 0x4000,

	all = 0x7FFF
};

enum class PlayerCharacter {
	flandre = 0,
	rumia,
	cirno,

	end
};

//Layers are interpreted as a bitmask.
//For now, multilayer physics is not being used.
enum class PhysicsLayers{
	none = 0,
	//For objects which are in/on the floor, including the floor itself
	//(FloorSegment), and objects touching the floor.
	belowFloor = 1,
	floor = 2, 
    ground = 4,
    eyeLevel = 8,
    
    //must be the bitwise or of all layers
    all = 15
};

enum class TimerType {
	begin = 0,
	gobject = 0,
	draw,
	physics,

	end
};

#endif /* types_h */
