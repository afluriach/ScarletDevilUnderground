//
//  Meiling.hpp
//  Koumachika
//
//  Created by Toni on 2/28/19.
//
//

#ifndef Meiling_hpp
#define Meiling_hpp

#include "Agent.hpp"

class Meiling1 : virtual public Agent, public NoAttributes, public DialogEntity
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	Meiling1(GSpace* space, ObjectIDType id, const ValueMap& args);
    
    virtual inline SpaceFloat getMass() const { return -1.0; }
    virtual inline GType getType() const { return GType::npc; }
    
    virtual inline string imageSpritePath() const { return "sprites/meiling.png"; }
    virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual inline bool isDialogAvailable() { return true; }
	virtual string getDialog();

	virtual inline void initStateMachine(ai::StateMachine& sm) {}
};

#endif /* Meiling_hpp */
