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
    right=1,
    up,
    left,
    down,
    none,
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
	none,
};

//Layers are interpreted as a bitmask.
//For now, multilayer physics is not being used.
enum class PhysicsLayers{
    floor = 1,
    ground = 2,
    
    //must be the bitwise or of all layers
    all = 3
};

#endif /* types_h */
