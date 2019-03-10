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

class MarisaNPC : virtual public Agent, public NoAttributes, public DialogEntity
{
public:
	MapObjCons(MarisaNPC);

	virtual inline GType getType() const { return GType::npc; }
	virtual inline SpaceFloat getRadius() const { return 0.35; }
	inline SpaceFloat getMass() const { return -1.0; }
	inline string imageSpritePath() const { return "sprites/marisa.png"; }

	inline virtual bool isDialogAvailable() { return true; }
	inline virtual string getDialog() { return "dialogs/marisa1"; }
};

class Marisa : public Enemy, public BaseAttributes<Marisa>
{
public:
	static const AttributeMap baseAttributes;

	MapObjCons(Marisa);
    
    virtual inline SpaceFloat getRadarRadius() const {return 6.0;}
    
    virtual inline SpaceFloat getRadius() const {return 0.35;}
    inline SpaceFloat getMass() const {return 33.0;}
    
    inline string imageSpritePath() const {return "sprites/marisa.png";}
};

class CollectMarisa : public Marisa {
public:
	MapObjCons(CollectMarisa);
	virtual void initStateMachine(ai::StateMachine& sm);
};

class ForestMarisa : public Marisa {
public:
	MapObjCons(ForestMarisa);
	virtual void initStateMachine(ai::StateMachine& sm);
};

class MarisaCollectMain : public ai::Function {
public:
	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	FuncGetName(MarisaCollectMain)
};

class MarisaForestMain : public ai::Function {
public:
	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	FuncGetName(MarisaForestMain)
};

#endif /* Marisa_h */
