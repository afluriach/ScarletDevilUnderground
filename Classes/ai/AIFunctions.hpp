//
//  AIFunctions.hpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#ifndef AIFunctions_hpp
#define AIFunctions_hpp

#include "AI.hpp"

namespace ai{

class MoveToPoint;

class ScriptFunction : public Function {
public:
	ScriptFunction(GObject* object, const string& cls);
	ScriptFunction(GObject* object, GObject* target, const string& cls);

	virtual void onEnter();
	virtual void update();
	virtual void onExit();

	 virtual string getName();
protected:
	bool hasMethod(const string& name);

	string cls;
	sol::table obj;
};

class AgentFunction : public Function {
public:
    AgentFunction(GObject* object);
protected:
    Agent *const agent;
};

class PlayerFunction : public Function {
public:
    PlayerFunction(GObject* object);
protected:
    Player *const player;
};

class Seek : public Function {
public:
	static constexpr double pathfindingCooldown = 0.25;

	enum class states {
		direct_seek,
		pathfinding,
		arriving,
		no_target,
	};

	Seek(GObject* object, GObject* target, bool usePathfinding, SpaceFloat margin = 0.0);
    
	virtual void update();
    
	FuncGetName(Seek)
protected:
	local_shared_ptr<Function> pathFunction;
	gobject_ref target;
	SpaceFloat margin;
	SpaceFloat lastPathfind = 0.0;
	states crntState = states::direct_seek;
	bool usePathfinding;
};

class MaintainDistance : public Function {
public:
    MaintainDistance(GObject* object, GObject* target, SpaceFloat distance, SpaceFloat margin);
    
	virtual void update();
    FuncGetName(MaintainDistance)
protected:
	gobject_ref target;
    SpaceFloat distance, margin;
};

class OccupyPoint : public Function {
public:
	OccupyPoint(GObject* object, SpaceVect target);

	virtual void update();

	FuncGetName(OccupyPoint)
protected:
	SpaceVect target;
};

class OccupyMidpoint : public Function {
public:
	OccupyMidpoint(GObject* object, GObject* target1, GObject* target2);

	virtual void update();

	FuncGetName(OccupyMidpoint)
protected:
	gobject_ref target1, target2;
};

class Scurry : public Function {
public:
	Scurry(
		GObject* object,
		GObject* _target,
		SpaceFloat distance,
		SpaceFloat length
	);

	virtual void update();

	FuncGetName(Scurry)
protected:
	local_shared_ptr<MoveToPoint> moveFunction;
	unsigned int startFrame, endFrame;
	SpaceFloat distance;
	bool scurryLeft = true;
	gobject_ref target;
};

class Flee : public Function {
public:
	Flee(GObject* object, GObject* target, SpaceFloat distance);
    
	virtual void update();

	FuncGetName(Flee)
protected:
	gobject_ref target;
    SpaceFloat distance;
};

class Evade : public AgentFunction {
public:
	Evade(GObject* object, GType type);

	virtual void update();

	FuncGetName(Evade)
protected:
	GType type;
	bool active = false;
};

class LookAround : public Function {
public:
	LookAround(GObject* object, SpaceFloat angularVelocity);

	virtual void update();

	FuncGetName(LookAround)
private:
	SpaceFloat angularVelocity;
};

class Flank : public Function {
public:
	Flank(
		GObject* object,
		GObject* target,
		SpaceFloat desiredDistance,
		SpaceFloat wallMargin
	);

	virtual void onEnter();
	virtual void update();
	FuncGetName(Flank);

	bool wallQuery(SpaceVect pos);
private:
	local_shared_ptr<MoveToPoint> moveFunction;
	gobject_ref target;
	SpaceFloat desiredDistance;
	SpaceFloat wallMargin;
};

class LookTowardsFire : public AgentFunction {
public:
	static constexpr float hitCost = 0.375f;
	static constexpr float lookThresh = 1.0f;
	static constexpr float timeCoeff = 0.5f;
	static constexpr float lookTimeCoeff = 0.125f;

	LookTowardsFire(GObject* object, bool useShield);

	virtual void onEnter();
	virtual void update();
	virtual void onExit();

	virtual bool bulletHit(Bullet* b);

	FuncGetName(LookTowardsFire)
protected:
	unsigned int hitCallbackID = 0;
	unsigned int blockCallbackID = 0;
	float hitAccumulator = 0.0f;
	SpaceVect directionAccumulator;
	bool useShield;
	bool looking = false;
};

class MoveToPoint : public Function{
public:
	static const SpaceFloat arrivalMargin;

	MoveToPoint(GObject* object, SpaceVect target);
    
	virtual void update();

    FuncGetName(MoveToPoint)
protected:
    SpaceVect target;
};

enum class follow_path_mode
{
	none = 0x0,
	loop = 0x1,
	scan = 0x2,
	stopForObstacle = 0x4,
};

class FollowPath : public Function {
public:
	static local_shared_ptr<FollowPath> pathToTarget(
		GObject* object,
		GObject* target
	);
	static local_shared_ptr<FollowPath> pathToPoint(
		GObject* object,
		SpaceVect point
	);

	FollowPath(GObject* object, shared_ptr<const Path> path, follow_path_mode mode);
	inline virtual ~FollowPath() {}

	virtual void update();
	FuncGetName(FollowPath)
protected:
	shared_ptr<const Path> path;
	size_t currentTarget = 0;
	follow_path_mode mode;
};

class FollowPathKinematic : public Function {
public:
	static local_shared_ptr<FollowPathKinematic> pathToPoint(
		GObject* object,
		SpaceVect point
	);

	FollowPathKinematic(
        GObject* object,
        shared_ptr<const Path> path,
        follow_path_mode mode
    );
	inline virtual ~FollowPathKinematic() {}

    void setSegment(size_t idx1, size_t idx2);
    void nextSegment();

    virtual void onEnter();
	virtual void update();
	FuncGetName(FollowPathKinematic)
protected:
	shared_ptr<const Path> path;
    SpaceVect currentSegmentStart;
    SpaceVect currentSegmentDisplacementNormal;
    SpaceFloat d = 0.0;
    SpaceFloat currentSegmentLength = 0.0;
    int idx1 = 0;
    int idx2 = 1;
    follow_path_mode mode;
};

class Wait : public Function {
public:
    Wait(GObject* object, SpaceFloat duration);
    
    virtual void update();
    FuncGetName(FollowPath)
protected:
    SpaceFloat duration;
    SpaceFloat t = 0.0;
};

class Wander : public Function {
public:
	Wander(GObject* object, SpaceFloat minWait, SpaceFloat maxWait, SpaceFloat minDist, SpaceFloat maxDist);
	Wander(GObject* object, SpaceFloat waitInterval, SpaceFloat moveDist);
	Wander(GObject* object);

	pair<Direction, SpaceFloat> chooseMovement();

	virtual void update();
	virtual void reset();

	FuncGetName(Wander)
protected:
	local_shared_ptr<MoveToPoint> moveFunction;
    SpaceFloat minWait, maxWait;
    SpaceFloat minDist, maxDist;
	SpaceFloat waitTimer = 0.0;
};

class FireOnStress : public AgentFunction {
public:
	FireOnStress(GObject* object, float stressPerShot);

	virtual void update();

	FuncGetName(FireOnStress)
protected:
	float stressPerShot;
};

class FireAtTarget : public AgentFunction {
public:
	FireAtTarget(GObject* object, GObject* target);

	virtual void update();

	FuncGetName(FireAtTarget)
protected:
	gobject_ref target;
};

class ThrowBombs : public AgentFunction {
public:
	ThrowBombs(
		GObject* object,
		GObject* target,
		local_shared_ptr<bomb_properties> bombType,
		SpaceFloat throwingSpeed,
		SpaceFloat baseInterval
	);

	virtual void onEnter();
	virtual void update();

	SpaceFloat getInterval();
	float score(SpaceVect pos, SpaceFloat angle);

	FuncGetName(ThrowBombs)
protected:
	local_shared_ptr<bomb_properties> bombType;

	SpaceFloat countdown;
	SpaceFloat throwingSpeed;
	SpaceFloat baseInterval;
	gobject_ref target;
};

class PlayerControl : public PlayerFunction {
public:
	PlayerControl(GObject* object);
    virtual ~PlayerControl();

	virtual void onEnter();
	virtual void update();
     
    void checkBlockControls(const ControlInfo& cs);
    void checkMovementControls(const ControlInfo& cs);
	void checkFireControls(const ControlInfo& cs);
	void checkBombControls(const ControlInfo& cs);
	void checkItemInteraction(const ControlInfo& cs);
    void updateSpellControls(const ControlInfo& cs);
    
    void applyDesiredMovement();
    bool tryInteract(GObject* interactible);
    
    void toggleSpell();
    void stopSpell();

	FuncGetName(PlayerControl)
protected:
    local_shared_ptr<Spell> activeSpell;
    SpaceVect desiredMoveDirection;
    SpaceFloat interactCooldown = 0.0;

	bool isAutoFire = false;
};

} //end NS

#endif /* AIFunctions_hpp */
