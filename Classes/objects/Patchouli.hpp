//
//  Patchouli.hpp
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#ifndef Patchouli_hpp
#define Patchouli_hpp

#include "Enemy.hpp"
#include "NPC.hpp"

class Patchouli : public NPC
{
public:
	Patchouli(GSpace* space, ObjectIDType id, const ValueMap& args);
    
    inline string getSprite() const {return "patchouli";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual bool isDialogAvailable() { return true; }
	virtual string getDialog();
	virtual void onDialogEnd();
};

#endif /* Patchouli_hpp */
