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

class NPC;

struct dialog_entry
{
	dialog_entry();
	dialog_entry(string dialog);
	dialog_entry(
		function<bool(NPC*)> condition,
		function<void(NPC*)> effect,
		string dialog,
		bool once
	);

	function<bool(NPC*)> condition;
	function<void(NPC*)> effect;

	string dialog;
	bool once = false;
};

class npc_properties : public agent_properties
{
public:
	inline npc_properties() {}
	~npc_properties();

	list<local_shared_ptr<dialog_entry>> dialogs;

	inline virtual type_index getType() const { return typeid(*this); }
};

class NPC : public Agent
{
public:
	NPC(
		GSpace* space,
		ObjectIDType id,
		const agent_attributes& attr,
		local_shared_ptr<npc_properties> props
	);

	~NPC();

	//provides an interface to make an agent an interactible object with dialog.
	virtual bool canInteract(Player* p);
	virtual void interact(Player* p);
	virtual string interactionIcon(Player* p);
protected:
	bool isDialogAvailable();
	local_shared_ptr<dialog_entry> getDialog();
	void onDialogEnd();

	local_shared_ptr<npc_properties> props;
	local_shared_ptr<dialog_entry> crntDialog;
};

#endif
