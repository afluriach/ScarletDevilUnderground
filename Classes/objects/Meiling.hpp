//
//  Meiling.hpp
//  Koumachika
//
//  Created by Toni on 2/28/19.
//
//

#ifndef Meiling_hpp
#define Meiling_hpp

#include "NPC.hpp"

class Meiling1 : public NPC
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	Meiling1(GSpace* space, ObjectIDType id, const ValueMap& args);
    
    virtual inline string getSprite() const { return "meiling"; }
    virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual inline bool isDialogAvailable() { return true; }
	virtual string getDialog();
};

#endif /* Meiling_hpp */
