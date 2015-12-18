//
//  BlockScene.h
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#ifndef BlockScene_h
#define BlockScene_h

#include "PlayScene.hpp"

class BlockScene : public PlayScene
{
public:
    inline BlockScene() : PlayScene("maps/block_room.tmx"){
    }

    CREATE_FUNC(BlockScene);
};

#endif /* BlockScene_h */
