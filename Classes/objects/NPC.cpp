//
//  NPC.cpp
//  Koumachika
//
//  Created by Toni on 6/25/19.
//
//

#include "Prefix.h"

#include "LuaAPI.hpp"
#include "NPC.hpp"

dialog_entry::dialog_entry() {}

dialog_entry::dialog_entry(string dialog) :
	dialog(dialog)
{}

dialog_entry::dialog_entry(
	function<bool(NPC*)> condition,
	function<void(NPC*)> effect,
	string dialog,
	bool once
) :
	condition(condition),
	effect(effect),
	dialog(dialog),
	once(once)
{}

npc_properties::~npc_properties()
{
}

NPC::NPC(
	GSpace* space,
	ObjectIDType id,
	const object_params& params,
	local_shared_ptr<npc_properties> props
) : 
	Agent(
		space,
		id,
		enum_bitwise_or(GType, npc, interactible),
		params,
		props
	),
	props(props)
{
    auto objects = space->scriptVM->_state["objects"];
	auto cls = objects[props->clsName];
    if (cls.valid()) {
		scriptObj = cls(this);
	}
}

NPC::~NPC()
{
}

bool NPC::canInteract(Player* p)
{
	return isDialogAvailable();
}

void NPC::interact(Player* p)
{
	if (crntDialog) {
		log1("NPC %s attempt to start dialog when one is already active!", getName());
		return;
	}

	auto _dialog = getDialog();

	if (_dialog) {
		crntDialog = _dialog;
		space->createDialog(
			string("dialogs/") + _dialog->dialog,
			false,
			[this]()->void {onDialogEnd(); }
		);
	}
}

string NPC::interactionIcon(Player* p) {
	return "sprites/ui/dialog.png";
}

local_shared_ptr<dialog_entry> NPC::getDialog()
{
	for (auto dialog : props->dialogs) {
		bool noReplay = dialog->once && App::crntState->hasCompletedDialog(dialog->dialog);
		bool condition = !dialog->condition || dialog->condition(this);

		if (!noReplay && condition)
			return dialog;
	}

	return nullptr;
}

void NPC::onDialogEnd()
{
	if (crntDialog) {
		if (crntDialog->effect) {
			crntDialog->effect(this);
		}

		App::crntState->dialogs.insert(crntDialog->dialog);

		crntDialog.reset();
	}
}

bool NPC::isDialogAvailable()
{
	return props->dialogs.size() > 0;
}
