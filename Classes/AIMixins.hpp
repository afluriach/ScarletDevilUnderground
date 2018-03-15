//
//  AIMixins.hpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#ifndef AIMixins_hpp
#define AIMixins_hpp

class StateMachineObject : virtual public GObject, RegisterUpdate<StateMachineObject>
{
public:
	inline StateMachineObject(shared_ptr<ai::State> startState) :
		RegisterUpdate(this),
		fsm(this)
	{
		fsm.push(startState);
	}

	inline void update() {
		fsm.update();
	}

protected:
	ai::StateMachine fsm;
};

class RadarStateMachineObject : virtual public GObject, StateMachineObject, RadarObject
{
public:
	inline RadarStateMachineObject(shared_ptr<ai::State> startState) : StateMachineObject(startState) {}

	inline virtual void onDetect(GObject* obj) {
		fsm.onDetect(obj);
	}

	inline virtual void onEndDetect(GObject* obj) {
		fsm.onEndDetect(obj);
	}
};

#endif /* AIMixins_hpp */
