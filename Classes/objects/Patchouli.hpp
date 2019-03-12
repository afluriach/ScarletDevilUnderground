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

class Patchouli : virtual public Agent, public NoAttributes, public DialogEntity
{
public:
	Patchouli(GSpace* space, ObjectIDType id, const ValueMap& args);
    
    inline SpaceFloat getMass() const {return -1.0;}
    virtual inline GType getType() const {return GType::npc;}
    
    inline string imageSpritePath() const {return "sprites/patchouli.png";}
    inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual bool isDialogAvailable() { return true; }
	virtual string getDialog();
	virtual void onDialogEnd();

	virtual void initStateMachine(ai::StateMachine& sm);
};

class PatchouliEnemy : public Enemy, public BaseAttributes<PatchouliEnemy>
{
public:
	static const AttributeMap baseAttributes;
	static const vector<float_pair> intervals;
	static const vector<SpellGeneratorType> spells;

	PatchouliEnemy(GSpace* space, ObjectIDType id, const ValueMap& args);

	inline SpaceFloat getMass() const { return 30.0; }

	inline string imageSpritePath() const { return "sprites/patchouli.png"; }
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void initStateMachine(ai::StateMachine& sm);
};

class PatchouliMain : public ai::Function {
public:
	static const int castInterval;

	virtual void onEnter(ai::StateMachine& sm);
	virtual void update(ai::StateMachine& sm);
	FuncGetName(PatchouliMain)
protected:
	float prevHP;
	size_t spellIdx = 0;
};

#endif /* Patchouli_hpp */
