//
//  types.h
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef types_h
#define types_h

class GObject;
class GSpace;

typedef pair<int,int> IntVec2;
//Represents a Chipmunk physics unit space vector
typedef cp::Vect SpaceVect;
typedef cp::Float SpaceFloat;
typedef vector<SpaceVect> Path;

typedef unsigned int ObjectIDType;
typedef function<GObject*(GSpace*, ObjectIDType)> ObjectGeneratorType;

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
	overhead,
	roomMask,

	end
};

enum class GType{
    player=1,
    playerBullet,
    enemy,
    enemyBullet,
	enemyBulletRadar,
    environment,
    foliage,
    wall,
	effectArea,
    playerSensor,
    objectSensor,
    collectible,
    npc,
	floorSegment,
	none,
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
	floor = 1, 
    ground = 2,
    eyeLevel = 4,
    
    //must be the bitwise or of all layers
    all = 7
};

struct PhysicsProperties
{
	bool setPos, setVel, setAngle, setAngularVel, setLayers;

	SpaceVect pos, vel;
	SpaceFloat angle, angularVel;
	PhysicsLayers layers;

	inline PhysicsProperties() :
		setPos(false), setVel(false), setAngle(false), setAngularVel(false), setLayers(false),
		pos(SpaceVect::zero), vel(SpaceVect::zero),
		angle(0.0f), angularVel(0.0f),
		layers(PhysicsLayers::none)
	{}
};

#endif /* types_h */
