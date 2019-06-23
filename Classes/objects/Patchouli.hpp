//
//  Patchouli.hpp
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#ifndef Patchouli_hpp
#define Patchouli_hpp

#include "Agent.hpp"
#include "Enemy.hpp"

class Patchouli : virtual public Agent, public DialogEntity
{
public:
	Patchouli(GSpace* space, ObjectIDType id, const ValueMap& args);
    
    inline SpaceFloat getMass() const {return -1.0;}
    virtual inline GType getType() const {return GType::npc;}
    
    inline string getSprite() const {return "patchouli";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual bool isDialogAvailable() { return true; }
	virtual string getDialog();
	virtual void onDialogEnd();
};

class PatchouliEnemy : public Enemy, public BaseAttributes<PatchouliEnemy>
{
public:
	static const string baseAttributes;
	static const vector<float_pair> intervals;
	static const vector<SpellGeneratorType> spells;

	PatchouliEnemy(GSpace* space, ObjectIDType id, const ValueMap& args);

	inline SpaceFloat getMass() const { return 30.0; }

	inline string getSprite() const { return "patchouli"; }
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	inline virtual string initStateMachine() { return "patchouli_enemy"; }
};

#endif /* Patchouli_hpp */
