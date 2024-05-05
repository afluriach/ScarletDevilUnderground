//
//  AI.hpp
//  Koumachika
//
//  Created by Toni on 12/13/15.
//
//

#ifndef AI_hpp
#define AI_hpp

namespace ai{

#define FuncGetName(cls) inline virtual string getName() const {return #cls;}

class Function
{
public:
	enum class state
	{
		created,
		active,
		completing,
		completed,
	};

	template<class C>
	inline static void autoUpdateFunction(local_shared_ptr<C>& f) {
		if (f && !f->isCompleted())
			f->update();
		if (f && f->isCompleted())
			f.reset();
	}

	Function(GObject* object);
	virtual ~Function();

	GSpace* getSpace() const;
	GObject* getObject() const;
	physics_context* getPhys() const;
 
	bool isActive() const;
	bool isCompleted() const;

	inline virtual void onEnter() {}
	inline virtual void update() { }
	inline virtual void onExit() {}
 
	void runEnter();
	void runUpdate();
	void runExit();

	inline virtual string getName() const {return "Function";}
	
	GObject* const object;
	state _state = state::created;
	int _refcount = 0;
protected:
};

} //end NS

#endif /* AI_hpp */
