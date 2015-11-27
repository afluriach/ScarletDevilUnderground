//
//  LibraryOpening.h
//  FlansBasement
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
    const int edgeMargin = 30;
        
    CREATE_FUNC(LibraryOpening);
    
    inline virtual bool init()
    {
        Layer::init();
        
        Sprite* background = Sprite::create("portraits/library1.png");
        background->setScale(0.2);
        background->setPosition(App::getScreenCenter());
        addChild(background, 1);
        
        dialogNode = Dialog::create();
        dialogNode->setDialog("dialogs/library_opening");
        dialogNode->setPosition(dialogPosition());
        addChild(dialogNode, 2);

        return true;
    }
    
    inline Vec2 dialogPosition()
    {
        return Vec2(App::width/2, Dialog::height/2 + edgeMargin);
    }
    
private:
    Dialog* dialogNode;
    
    const std::vector<DialogFrame> dialog = list_of_typed(
        (setText("“Finally, I made it to the library again.” Flandre looked around wide-eyed at the library."))
        (setText("She took off running.")),
        std::vector<DialogFrame>
    );
};

#endif /* LibraryOpening_h */
