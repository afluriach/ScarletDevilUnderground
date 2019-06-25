//
//  GObjectMixins.hpp
//  Koumachika
//
//  Created by Toni on 4/15/18.
//
//

#ifndef GObjectMixins_hpp
#define GObjectMixins_hpp

#include "functional.hpp"
#include "GObject.hpp"
#include "Graphics.h"
#include "multifunction.h"

class Agent;
class PatchConAnimation;
class TimedLoopAnimation;
class Wall;

//LOGIC MIXINS

class DialogEntity : public virtual GObject
{
public:
	inline DialogEntity() {}

    virtual bool isDialogAvailable() = 0;
    virtual string getDialog() = 0;
	inline virtual void onDialogEnd() {}

    inline virtual bool canInteract(Player* p){
        return isDialogAvailable();
    }
    virtual void interact(Player* p);

    inline virtual string interactionIcon(Player* p){
        return "sprites/ui/dialog.png";
    }
};

class DialogImpl : public DialogEntity
{
public:
	DialogImpl(const ValueMap& args);

	inline virtual bool isDialogAvailable() { return !dialogName.empty(); }
	inline virtual string getDialog() { return "dialogs/"+dialogName; }
protected:
	string dialogName;
};

//END LOGIC

#endif /* GObjectMixins_hpp */
