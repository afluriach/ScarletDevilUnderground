//
//  NPC.hpp
//  Koumachika
//
//  Created by Toni on 6/25/19.
//
//

#ifndef NPC_hpp
#define NPC_hpp

class NPC : public Agent
{
public:
	NPC(
		GSpace* space,
		ObjectIDType id,
		const object_params& params,
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
