//
//  Collect.cpp
//  Koumachika
//
//  Created by Toni on 12/15/18.
//
//

#include "Prefix.h"

#include "Collect.h"
#include "functional.hpp"
#include "GSpace.hpp"

const vector<string> Collect::targets = {
	"glyph1",
	"glyph2",
	"glyph3",
	"glyph4",
	"glyph5"
};

Collect::Collect() :
PlayScene("Collect")
{
	multiInit.insertWithOrder(
		wrap_method(Collect, initTargets, this),
		static_cast<int>(initOrder::postLoadObjects)
	);
}

void Collect::initTargets()
{
	for (const string& tname : targets) {
		activationTargets.insert(gspace->getObjectRef(tname));
	}
}

void Collect::registerActivation(gobject_ref target)
{
	activationTargets.erase(target);

	if (activationTargets.empty()) {
		triggerSceneCompleted();
	}
}