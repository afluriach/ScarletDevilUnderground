//
//  CollectScene.h
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#ifndef CollectScene_h
#define CollectScene_h

#include "PlayScene.hpp"

class Collect : public PlayScene
{
public:
    inline Collect() :
    PlayScene("Collect")
    {
        createDialog("dialogs/collect_opening", true);
    }

    CREATE_FUNC(Collect);
};

#endif /* CollectScene_h */
