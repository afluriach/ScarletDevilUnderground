//
//  Reimu.hpp
//  Koumachika
//
//  Created by Toni on 3/15/16.
//
//

#ifndef Reimu_h
#define Reimu_h

#include "Enemy.hpp"
#include "NPC.hpp"

class Reimu : public NPC
{
public:
	MapObjCons(Reimu);
    
    virtual inline string getSprite() const {return "reimu";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    inline virtual bool isDialogAvailable(){ return true;}
    inline virtual string getDialog(){
        return "dialogs/warning_about_rumia";
    }
};

class ReimuEnemy : public Enemy
{
public:
	static const string baseAttributes;
	static const string properName;

	MapObjCons(ReimuEnemy);

	virtual inline SpaceFloat getRadarRadius() const { return 5.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return 0.0; }

	virtual inline string getTypeName() const { return "ReimuEnemy"; }

	virtual inline string getSprite() const { return "reimu"; }
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual inline string initStateMachine() { return "reimu_enemy"; }
};

#endif /* Reimu_h */
