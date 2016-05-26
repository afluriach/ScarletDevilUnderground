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

enum Direction{
    rightDir=1,
    upDir,
    leftDir,
    downDir,
    noneDir,
};

enum GraphicsLayer{
    map = 1,
    foliageLayer,
    floorGraphicsLayer,
    ground,
};

enum GType{
    player=1,
    playerBullet,
    enemy,
    enemyBullet,
    environment,
    foliage,
    wall,
    playerSensor,
    objectSensor,
};

//Layers are interpreted as a bitmask.
//For now, multilayer physics is not being used.
enum PhysicsLayers{
    floorPhysicsLayer = 1,
    groundLayer = 2,
    
    //must be the bitwise or of all layers
    allLayers = 3
};

#endif /* types_h */
