//
//  Sign.cpp
//  Koumachika
//
//  Created by Toni on 2/17/19.
//
//

#include "Prefix.h"

#include "Sign.hpp"
#include "value_map.hpp"

Sign::Sign(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParamsPointUp(), MapRectPhys(GType::environment, -1.0))
{
	dialogRes = getStringOrDefault(args, "dialog", "");
}

bool Sign::canInteract(Player* p)
{
	return !dialogRes.empty();
}

void Sign::interact(Player* p)
{
	if (!dialogRes.empty()) {
		space->createDialog(
			"dialogs/" + dialogRes,
			false
		);
	}
}

string Sign::interactionIcon(Player* p)
{
	return "sprites/ui/dialog.png";
}
