//
//  Agent.hpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#ifndef Agent_hpp
#define Agent_hpp

#include "AI.hpp"
#include "AIMixins.hpp"
#include "GObject.hpp"

class Agent : virtual public GObject, public PatchConSprite, public CircleBody, public StateMachineObject, public RadarObject, public Spellcaster, RegisterInit<Agent>
{
public:
	inline Agent(const ValueMap& args) :
		GObject(args),
		PatchConSprite(args),
		StateMachineObject(args),
		RegisterInit<Agent>(this)
	{}

	inline void init() {
		shared_ptr<ai::State> startState = getStartState();

		if (startState) {
			fsm.push(startState);
		}
	}

	//replaces functionality of RadarStateMachineObject by connecting sensor callbacks
	inline virtual void onDetect(GObject* obj) {
		fsm.onDetect(obj);
		RadarObject::onDetect(obj);
	}

	inline virtual void onEndDetect(GObject* obj) {
		fsm.onEndDetect(obj);
		RadarObject::onEndDetect(obj);
	}

	//sensor interface
	virtual float getRadarRadius() const { return 1.0f; }
	virtual GType getRadarType() const { return GType::none; }
	virtual float getDefaultFovAngle() const { return 0.0f; }

	//physics/motor interface
	virtual inline float getRadius() const { return 0.35f; }
	virtual float getMass() const = 0;
	virtual GType getType() const = 0;

	//graphics interface
	inline string imageSpritePath() const = 0;
	inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	//AI interface
	virtual inline shared_ptr<ai::State> getStartState() { return nullptr; }
};

class GenericAgent : public Agent
{
public:
    inline GenericAgent(const ValueMap& args) :
	GObject(args),
	Agent(args)
    {
        spriteName = args.at("sprite").asString();
    }
    
    virtual inline float getRadarRadius() const {return 3.0f;}
    virtual inline GType getRadarType() const { return GType::playerSensor;}
    virtual inline float getDefaultFovAngle() const {return 0.0f;}
    
    //virtual inline float getRadius() const {return 0.35f;}
    inline float getMass() const {return 20.0f;}
    virtual inline GType getType() const {return GType::enemy;}
    
    virtual inline float getMaxSpeed() const {return 1.5f;};
    virtual inline float getMaxAcceleration() const {return 6.0f;}
    
    inline string imageSpritePath() const {return "sprites/"+spriteName+".png";}
    //inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

	virtual inline shared_ptr<ai::State> getStartState() {
		return make_shared<ai::WanderAndFleePlayer>(3.0f, this);
	}
protected:
    string spriteName;
};

#endif /* Agent_hpp */
