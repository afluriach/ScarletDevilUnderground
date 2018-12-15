//
//  Reimu.hpp
//  Koumachika
//
//  Created by Toni on 3/15/16.
//
//

#ifndef Reimu_h
#define Reimu_h

#include "Agent.hpp"

class Reimu : virtual public Agent, public DialogEntity, public NoAttributes
{
public:
    inline Reimu(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
    MapObjForwarding(Agent)
    {}
    
    virtual inline SpaceFloat getRadius() const {return 0.35;}
    inline SpaceFloat getMass() const {return -1.0;}
    virtual inline GType getType() const {return GType::npc;}
    
    inline string imageSpritePath() const {return "sprites/reimu.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    inline virtual bool isDialogAvailable(){ return true;}
    inline virtual string getDialog(){
        return "dialogs/warning_about_rumia";
    }
};

#endif /* Reimu_h */
