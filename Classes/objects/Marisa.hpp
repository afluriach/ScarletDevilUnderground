//
//  Marisa.h
//  Koumachika
//
//  Created by Toni on 12/12/15.
//
//

#ifndef Marisa_h
#define Marisa_h

#include "Enemy.hpp"
#include "object_ref.hpp"

class MarisaNPC : virtual public Agent, public DialogEntity
{
public:
	MapObjCons(MarisaNPC);

	virtual inline GType getType() const { return GType::npc; }
	inline SpaceFloat getMass() const { return -1.0; }
	inline string getSprite() const { return "marisa"; }

	inline virtual bool isDialogAvailable() { return true; }
	inline virtual string getDialog() { return "dialogs/marisa1"; }
};

class Marisa : public Enemy, public BaseAttributes<Marisa>
{
public:
	static const string baseAttributes;
	static const string properName;

	MapObjCons(Marisa);
	inline virtual ~Marisa() {}
    
    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    
    inline SpaceFloat getMass() const {return 33.0;}
    
    inline string getSprite() const {return "marisa";}
};

class CollectMarisa : public Marisa {
public:
	MapObjCons(CollectMarisa);
	virtual inline string initStateMachine() { return "collect_marisa"; }
};

class ForestMarisa : public Marisa {
public:
	MapObjCons(ForestMarisa);
	virtual inline string initStateMachine() { return "forest_marisa"; }
};

#endif /* Marisa_h */
