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
#include "functional.hpp"
#include "GScene.hpp"
#include "multifunction.h"

class LibraryOpening : public GScene
{
public:    
    inline LibraryOpening() :
		GScene("LibraryOpening", {})
	{
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

	inline virtual GScene* getReplacementScene() {
		return Node::ccCreate<LibraryOpening>();
	}
};

#endif /* LibraryOpening_h */
