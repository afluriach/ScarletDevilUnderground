//
//  SakuyaNPC.hpp
//  Koumachika
//
//  Created by Toni on 12/27/18.
//
//

#ifndef SakuyaNPC_hpp
#define SakuyaNPC_hpp

#include "NPC.hpp"

class SakuyaNPC : public NPC
{
public:
	static const string baseAttributes;

	MapObjCons(SakuyaNPC);

	virtual inline GType getType() const { return GType::npc; }
    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    virtual inline SpaceFloat getDefaultFovAngle() const {return 0.0;}

    inline string getSprite() const {return "sakuya";}

	inline virtual string initStateMachine() { return "sakuya_npc"; }

	inline virtual bool isDialogAvailable() { return true; }
	inline virtual string getDialog() { return "dialogs/sakuya1"; }
};

#endif /* SakuyaNPC_hpp */
