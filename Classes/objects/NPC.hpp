//
//  NPC.hpp
//  Koumachika
//
//  Created by Toni on 6/25/19.
//
//

#ifndef NPC_hpp
#define NPC_hpp

#include "Agent.hpp"

class NPC : public Agent
{
public:
	NPC(
		GSpace* space,
		ObjectIDType id,
		const ValueMap& args,
		const string& baseAttributes,
		SpaceFloat radius,
		SpaceFloat mass
	);

	//provides an interface to make an agent an interactible object with dialog.
	virtual bool canInteract(Player* p);
	virtual void interact(Player* p);
	virtual string interactionIcon(Player* p);

	//The base implementation will show a dialog interaction as available if dialog field is set.
	//Otherwise, override these to provide a dialog.
	virtual bool isDialogAvailable();
	virtual string getDialog();

	inline virtual void onDialogEnd() {}
protected:
	string dialog;
};

#endif
