//
//  AI.hpp
//  Koumachika
//
//  Created by Toni on 12/13/15.
//
//

#ifndef AI_hpp
#define AI_hpp

#include "sol_util.hpp"

namespace ai{

//for functions that target an object
typedef function<local_shared_ptr<Function>(StateMachine*, GObject*)> AITargetFunctionGenerator;

#define FuncGetName(cls) inline virtual string getName() const {return #cls;}

#define return_pop() return update_return(-1, 0.0f, nullptr)
#define return_push(x) return update_return(0, 0.0f, x)
#define _steady(x) update_return(0, x, nullptr)
#define return_steady(x) return _steady(x)

struct update_return
{
	static update_return makePush(local_shared_ptr<Function> f);
	static update_return makePop();
	static update_return makeSteady(float update);

	update_return();
	update_return(int idx, float update, local_shared_ptr<Function> f);

	int idx;
	float update;
	local_shared_ptr<Function> f;

	bool isSteady();
	bool isPop();

	getter(int, idx);
	getter(float, update);
	getter(local_shared_ptr<Function>, f);
};

class Function
{
public:
	friend class Thread;

	template<class C>
	inline static void autoUpdateFunction(local_shared_ptr<C>& f) {
		if (f && !f->isCompleted())
			f->update();
		if (f && f->isCompleted())
			f.reset();
	}

	Function(StateMachine* fsm);
	virtual ~Function();

	inline StateMachine* getFSM() const { return fsm; }

	GSpace* getSpace() const;
	GObject* getObject() const;
	Agent* getAgent() const;
	physics_context* getPhys() const;

	bool castSpell(const SpellDesc* desc);
	local_shared_ptr<Spell> castSpellManual(const SpellDesc* desc);
	bool isSpellActive();
	void stopSpell();

	inline virtual void onEnter() {}
    inline virtual void onReturn() {}
	inline virtual void reset() {}
	inline virtual update_return update() { return_pop(); }
	inline virtual bool isActive() { return false; }
	inline virtual bool isCompleted() { return false; }
	inline virtual void onExit() {}

	inline virtual bool bulletBlock(Bullet* b) { return false; }
	inline virtual bool bulletHit(Bullet* b) { return false; }

	inline virtual bool detectEnemy(Agent* enemy) { return false; }
	inline virtual bool endDetectEnemy(Agent* enemy) { return false; }
	inline virtual bool detectBomb(Bomb* bomb) { return false; }
	inline virtual bool detectBullet(Bullet* bullet) { return false; }

	inline virtual bool enemyRoomAlert(Agent* enemy) { return false; }

	inline virtual bool zeroHP() { return false; }
	inline virtual bool zeroStamina() { return false; }

    inline virtual string getName() const {return "Function";}
    
	StateMachine* const fsm;
	int _refcount = 0;
protected:
	local_shared_ptr<Spell> spell;
};

class Thread
{
public:    
	friend class StateMachine;
    
	Thread(StateMachine* sm);
	~Thread();

	void update();

	void push(local_shared_ptr<Function> newState);
	void pop();
	void popToRoot();
    
	local_shared_ptr<Function> getTop();
    string getStack();
	string getMainFuncName();

	int _refcount = 0;
protected:
	list<local_shared_ptr<Function>> call_stack;
	StateMachine* sm;
};

class FSM
{
public:
    inline virtual ~FSM() {}
    virtual void update() = 0;
    
    virtual void onDetectEnemy(Agent* enemy) = 0;
	virtual void onEndDetectEnemy(Agent* enemy) = 0;
	virtual void onDetectBomb(Bomb* bomb) = 0;
	virtual void onDetectBullet(Bullet* bullet) = 0;
 
 	virtual void onBulletHit(Bullet* b) = 0;
	virtual void onBulletBlock(Bullet* b) = 0;
	virtual void enemyRoomAlert(Agent* enemy) = 0;
	virtual void onZeroHP() = 0;
	virtual void onZeroStamina() = 0;
 
    virtual string toString() = 0;
};

class StateMachine : public FSM
{
public:
	friend class Thread;

    StateMachine(GObject *const agent, const string& clsName);
	virtual ~StateMachine();

	virtual void update();

	void pushFunction(local_shared_ptr<Function> function);

	virtual void onDetectEnemy(Agent* enemy);
	virtual void onEndDetectEnemy(Agent* enemy);
	virtual void onDetectBomb(Bomb* bomb);
	virtual void onDetectBullet(Bullet* bullet);

	virtual void onBulletHit(Bullet* b);
	virtual void onBulletBlock(Bullet* b);
	virtual void enemyRoomAlert(Agent* enemy);
	virtual void onZeroHP();
	virtual void onZeroStamina();

	template<class FuncCls, typename... Params>
	inline void addFunction(Params... params) {
		addFunction(make_local_shared<FuncCls>(this, params...));
	}

	template<class FuncCls, typename... Params>
	inline local_shared_ptr<FuncCls> make(Params... params) {
		return make_local_shared<FuncCls>(this, params...);
	}
    
	GSpace* getSpace();
	inline GObject* getObject() { return agent; }
	Agent* getAgent();
	RoomSensor* getRoomSensor();
	unsigned int getFrame();
    string toString();
protected:
	template<typename... Params>
	void callInterface(const string& name, bool (Function::*method)(Params...), Params... params)
	{
		if (scriptObj && sol::hasMethod(scriptObj, name)) {
			auto f = scriptObj[name];
			bool result = f(scriptObj, params...);

			if (result) return;
		}

		for (auto it = _stack->call_stack.rbegin(); it != _stack->call_stack.rend(); ++it) {
			Function* f = it->get();
			if ((f->*method)(params...)) return;
		}
	}

	GObject *const agent;

	local_shared_ptr<Thread> _stack;
	sol::table scriptObj;
    unsigned int frame;
};

template<class FuncCls, typename... Params>
inline AITargetFunctionGenerator makeTargetFunctionGenerator(Params... params)
{
	return[params...](StateMachine* fsm, GObject* obj)->local_shared_ptr<Function> {
		return make_local_shared<FuncCls>(fsm, obj, params...);
	};
}

template<class FuncCls, typename... Params>
inline AITargetFunctionGenerator targetFunctionGeneratorAdapter(Params... params)
{
	return[params...](StateMachine* fsm, GObject* obj)->local_shared_ptr<Function> {
		return make_local_shared<FuncCls>(fsm, params...);
	};
}

} //end NS

#endif /* AI_hpp */
