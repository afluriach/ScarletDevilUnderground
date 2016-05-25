//
//  BlockRoom.h
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#ifndef BlockRoom_h
#define BlockRoom_h

#include "PlayScene.hpp"

class BlockRoom : public PlayScene
{
public:
    inline BlockRoom() : PlayScene("BlockRoom"){
    }

    CREATE_FUNC(BlockRoom);
};

#endif /* BlockRoom_h */
