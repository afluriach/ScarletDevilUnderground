//
//  NPC.cpp
//  Koumachika
//
//  Created by Toni on 6/25/19.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "NPC.hpp"

NPC::NPC(
	GSpace* space,
	ObjectIDType id,
	const ValueMap& args,
	const string& baseAttributes,
	SpaceFloat radius,
	SpaceFloat mass
) : 
	Agent(
		space, id, enum_bitwise_or(GType, npc, interactible), onGroundLayers, args,
		baseAttributes,
		radius,
		mass
	)
{
}

bool NPC::canInteract(Player* p)
{
	return isDialogAvailable();
}

void NPC::interact(Player* p)
{
	string _dialog = getDialog();

	if (!_dialog.empty()) {
		space->createDialog(
			getDialog(),
			false,
			[this]()->void {onDialogEnd(); }
		);
	}
}

string NPC::interactionIcon(Player* p) {
	return "sprites/ui/dialog.png";
}

bool NPC::isDialogAvailable()
{
	return !dialog.empty();
}

string NPC::getDialog()
{
	return dialog;
}
