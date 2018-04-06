//
//  LibraryOpening.h
//  Koumachika
//
//  Created by Toni on 11/16/15.
//
//

#ifndef LibraryOpening_h
#define LibraryOpening_h

#include "Dialog.hpp"

class LibraryOpening : public GScene
{
public:
    CREATE_FUNC(LibraryOpening);
    
    inline LibraryOpening(){
        multiInit += wrap_method(LibraryOpening,start, this);
    }
    
    inline void start()
    {
        Sprite* background = Sprite::create("portraits/library1.png");
        background->setScale(0.2f);
        background->setPosition(App::getScreenCenter());
        getLayer(sceneLayers::dialogBackground)->addChild(background);
        
        createDialog("dialogs/library_opening", false);
    }
};

#endif /* LibraryOpening_h */
