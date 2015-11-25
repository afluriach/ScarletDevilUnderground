//
//  types.h
//  FlansBasement
//
//  Created by Toni on 11/25/15.
//
//

#ifndef types_h
#define types_h

typedef std::pair<int,int> IntVec2;

enum Direction{
    rightDir=1,
    upDir,
    leftDir,
    downDir
};

enum GraphicsLayer{
    map = 1,
    ground,
};

enum GType{
    player=1,
    playerBullet,
    environment
};

//Layers are interpreted as a bitmask.
//For now, multilayer physics is not being used.
enum PhysicsLayers{
    groundLayer = 1,
};

#endif /* types_h */
