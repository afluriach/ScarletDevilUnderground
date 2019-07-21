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

typedef map<Attribute, float> AttributeMap;

typedef pair<float, float> float_pair;
typedef pair<double, double> double_pair;
typedef pair<int,int> IntVec2;
typedef tuple<long, long, long> TimerTriplet;
typedef map<type_index, pair<unsigned int, unsigned int>> EnemyStatsMap;
typedef vector<SpaceVect> Path;

typedef unsigned int LightID;
typedef unsigned int SpriteID;
typedef unsigned int ObjectIDType;

typedef function<void(void)> zero_arity_function;
typedef function<void(const vector<string>&)> InterfaceFunction;

typedef function<GObject*(GSpace*, ObjectIDType)> ObjectGeneratorType;
typedef function<ObjectGeneratorType(shared_ptr<object_params> )> ParamsGeneratorType;
typedef function<FiniteTimeAction*()> ActionGeneratorType;
typedef function<shared_ptr<Spell>(GObject*)> SpellGeneratorType;
typedef function<shared_ptr<FirePattern>(Agent*, int)> FirePatternGeneratorType;
//

typedef function<void(ai::StateMachine&, Player*)> alert_function;
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
	DamageInfo();
	DamageInfo(float mag, DamageType type);
	DamageInfo(float mag, Attribute element, DamageType type);

	float mag = 0.0f;
	//It is initialized to Attribute::end;
	Attribute element;
	DamageType type = DamageType::end;

	DamageInfo operator*(float rhs);
};

#define bullet_damage(x) DamageInfo(x, Attribute::end, DamageType::bullet)
#define bomb_damage(x) DamageInfo(x, Attribute::end, DamageType::bomb)
#define melee_damage(x) DamageInfo(x, Attribute::end, DamageType::melee)

struct app_params
{
	float getScale() const;

	unsigned int width = 1600;
	unsigned int height = 1000;

	bool fullscreen = false;
	bool vsync = true;
	bool multithread = true;
	bool showTimers = false;

	float difficultyScale = 1.0f;
	bool unlockAllEquips = false;

	unsigned int framesPerSecond = 60;
	double secondsPerFrame = 1.0 / framesPerSecond;
	boost::rational<int> secondsPerFrameRational = boost::rational<int>(1, framesPerSecond);
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

enum class ChamberID
{
	begin = 0,
	graveyard1 = begin,
	graveyard2,
	graveyard3,
	graveyard4,
	forest1,
	desert1,
	mine1,

	end,
	invalid_id = end
};

bool isValidChamber(ChamberID id);

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
    foliage = 0x20,
    wall = 0x40,
	areaSensor = 0x80,
    enemySensor = 0x100,
    playerGrazeRadar = 0x200,
    playerPickup = 0x400,
    npc = 0x800,
	floorSegment = 0x1000,
	bomb = 0x2000,

	all = 0x3FFF
};

extern const GType bombObstacles;
extern const GType bulletObstacles;
extern const GType agentObstacles;
extern const GType interactibleObjects;

enum class PlayerCharacter {
	flandre = 0,
	rumia,
	cirno,

	end
};

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
    
    //must be the bitwise or of all layers
    all = 15
};

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

#endif /* types_h */
