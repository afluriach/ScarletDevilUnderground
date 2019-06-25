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
#include "NPC.hpp"

class MarisaNPC : public NPC
{
public:
	MapObjCons(MarisaNPC);

	virtual inline GType getType() const { return GType::npc; }
	inline string getSprite() const { return "marisa"; }

	inline virtual bool isDialogAvailable() { return true; }
	inline virtual string getDialog() { return "dialogs/marisa1"; }
};

#endif /* Marisa_h */
