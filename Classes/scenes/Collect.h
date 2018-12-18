//
//  CollectScene.h
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#ifndef CollectScene_h
#define CollectScene_h

#include "object_ref.hpp"
#include "PlayScene.hpp"

class Collect : public PlayScene
{
public:
	static const vector<string> targets;
	Collect();

	void initTargets();
	void registerActivation(gobject_ref target);

	inline virtual GScene* getReplacementScene() { return Node::ccCreate<Collect>(); }
protected:
	set<gobject_ref> activationTargets;
};

#endif /* CollectScene_h */
