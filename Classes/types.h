//
//  types.h
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef types_h
#define types_h

class Agent;
namespace ai {
	class Function;
	class StateMachine;
	class Thread;
}
class Bullet;
class FirePattern;
class GObject;
class GSpace;
class object_params;
class Player;
class Spell;

enum class Attribute : int;

enum class GraphicsLayer : int;

typedef unordered_map<Attribute, float> AttributeMap;

typedef pair<float, float> float_pair;
typedef pair<double, double> double_pair;
typedef pair<int,int> IntVec2;
typedef tuple<long, long, long> TimerTriplet;
typedef map<string, pair<unsigned int, unsigned int>> EnemyStatsMap;
typedef vector<SpaceVect> Path;

typedef unsigned int LightID;
typedef unsigned int SpriteID;
typedef unsigned int ObjectIDType;

typedef function<void(void)> zero_arity_function;
typedef function<void(const vector<string>&)> InterfaceFunction;

typedef function<GObject*(GSpace*, ObjectIDType)> ObjectGeneratorType;
typedef function<FiniteTimeAction*()> ActionGeneratorType;
typedef function<local_shared_ptr<FirePattern>(Agent*)> FirePatternGeneratorType;
//

struct GraphicsAction
{
	ActionGeneratorType generator;

	inline GraphicsAction(ActionGeneratorType generator) : generator(generator) {}
};

typedef function<void(ai::StateMachine*, Player*)> alert_function;
typedef function<void(ai::StateMachine&, GObject*)> detect_function;
typedef function<void(GObject*)> unary_gobject_function;

typedef function<SpaceVect(SpaceFloat)> parametric_space_function;

constexpr size_t maxRoomsPerChamber = 16;
typedef bitset<maxRoomsPerChamber> rooms_bitmask;

constexpr size_t maxMapFragmentsPerChamber = 8;
typedef bitset<maxMapFragmentsPerChamber> map_fragments_bitmask;

enum class DamageType
{
	bullet,
	bomb,
	effectArea,
	touch,
	melee,
	pitfall,

	end
};

struct DamageInfo
{
	static DamageInfo bomb(float mag);
	static DamageInfo bullet(float mag);
	static DamageInfo melee(float mag);

	DamageInfo();
	DamageInfo(float mag, DamageType type);
	DamageInfo(float mag, DamageType type, Attribute element, float knockback);

	float mag = 0.0f;
	float knockback = 0.0f;
	//It is initialized to Attribute::end;
	Attribute element;
	DamageType type;

	DamageInfo operator*(float rhs);
	bool isExplosion();
	bool isValid();
};

struct app_params
{
	float getScale() const;
	double getFrameInterval() const;

	unsigned int width = 1600;
	unsigned int height = 1000;

	bool fullscreen = false;
	bool vsync = true;
	bool showTimers = false;

	float difficultyScale = 1.0f;
	bool unlockAllEquips = false;

	unsigned int framesPerSecond = 60;
	double secondsPerFrame = 1.0 / framesPerSecond;
};

struct floorsegment_properties {
	string sfxRes;
	double traction = 1.0;
};

struct sprite_properties {
	string filename;
	IntVec2 size;
	int dpi;
	float referenceSize = 0.0f;
	//only used for animation sequences
	float duration = 0.0;
	Color3B color = Color3B::BLACK;
};

namespace app {
	extern app_params params;
}

class SpaceRect
{
public:
	SpaceRect();
	SpaceRect(const b2AABB& box);
	SpaceRect(SpaceVect center, SpaceVect dimensions);
	SpaceRect(SpaceFloat llX, SpaceFloat llY, SpaceFloat width, SpaceFloat height);

	CCRect toPixelspace() const;

	double getMinX() const;
	double getMinY() const;
	double getMaxX() const;
	double getMaxY() const;

	SpaceVect getLLCorner() const;
	SpaceVect getURCorner() const;

	bool intersectsRect(const SpaceRect& rect) const;
	bool containsPoint(const SpaceVect& point) const;

	SpaceVect center, dimensions;
};

enum class Direction{
    none = 0,
    right,
    up,
    left,
    down,
    end,
};

enum class GType{
	none = 0x0,
	
	player = 0x1,
    playerBullet = 0x2,
    enemy = 0x4,
    enemyBullet = 0x8,
    environment = 0x10,
    wall = 0x20,
	areaSensor = 0x40,
    enemySensor = 0x80,
    playerGrazeRadar = 0x100,
    playerPickup = 0x200,
    npc = 0x400,
	floorSegment = 0x800,
	bomb = 0x1000,

	all = 0x1FFF,

	canDamage = 0x2000,
	interactible = 0x4000,
};

extern const GType bombObstacles;
extern const GType bulletObstacles;
extern const GType agentObstacles;

GType getBaseType(GType type);
GType parseType(string s);

//Layers are interpreted as a bitmask.
//For now, multilayer physics is not being used.
enum class PhysicsLayers{
	none = 0,
	//For objects which are in/on the floor, including the floor itself
	//(FloorSegment), and objects touching the floor.
	belowFloor = 1,
	floor = 2, 
    ground = 4,
    eyeLevel = 8,
    
	flying = ground,
	onGround  = floor + ground,
	eyeLevelHeight = floor + ground + eyeLevel,
    //must be the bitwise or of all layers
    all = 15
};

PhysicsLayers parseLayers(string s);

enum class TimerType {
	begin = 0,
	gobject = 0,
	draw,
	physics,

	end
};

enum class parametric_type {
	none = 0,

	position,
	velocity,
};

namespace sol {
	template <typename T>
	struct unique_usertype_traits<boost::local_shared_ptr<T>> {
		typedef T type;
		typedef boost::local_shared_ptr<T> actual_type;
		static const bool value = true;

		static bool is_null(const actual_type& ptr) {
			return ptr == nullptr;
		}

		static type* get(const actual_type& ptr) {
			return ptr.get();
		}
	};
}

#endif /* types_h */
