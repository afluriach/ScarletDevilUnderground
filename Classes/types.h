//
//  types.h
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef types_h
#define types_h

typedef pair<int,int> IntVec2;
//Represents a Chipmunk physics unit space vector
typedef cp::Vect SpaceVect;
typedef vector<SpaceVect> Path;

enum class Direction{
    none = 0,
    right,
    up,
    left,
    down,
    end,
};

enum class GraphicsLayer{
    map = 1,
    foliage,
    floor,
    ground,
};

enum class GType{
    player=1,
    playerBullet,
    enemy,
    enemyBullet,
    environment,
    foliage,
    wall,
    playerSensor,
    objectSensor,
    collectible,
    npc,
	none,
};

//Layers are interpreted as a bitmask.
//For now, multilayer physics is not being used.
enum class PhysicsLayers{
    floor = 1,
    ground = 2,
    eyeLevel = 4,
    
    //must be the bitwise or of all layers
    all = 7
};

struct PhysicsProperties
{
	bool setPos, setVel, setAngle, setAngularVel;

	SpaceVect pos, vel;
	float angle, angularVel;

	inline PhysicsProperties() :
		setPos(false), setVel(false), setAngle(false), setAngularVel(false),
		pos(SpaceVect::zero), vel(SpaceVect::zero),
		angle(0.0f), angularVel(0.0f)
	{}
};

#endif /* types_h */
