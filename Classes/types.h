//
//  types.h
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef types_h
#define types_h

//commonly used forward declarations
    //GObject types
class Agent;
class Bomb;
class Bullet;
class Door;
class EffectArea;
class Enemy;
class FloorSegment;
class GObject;
class NPC;
class Pitfall;
class Player;
class RadarSensor;
class RoomSensor;
class Wall;
    //object property, parameter types
class agent_properties;
struct bomb_properties;
struct bullet_attributes;
struct bullet_properties;
class effectarea_properties;
class enemy_properties;
class environment_object_properties;
class floorsegment_properties;
class item_properties;
class npc_properties;
class object_params;
class object_properties;
    //graphics, scene types
class AgentAnimationContext;
class AgentBodyShader;
class audio_context;
class ConeShader;
class Dialog;
class GAnimation;
class graphics_context;
class GScene;
class HUD;
class LightArea;
class LuaShell;
class MapMenu;
class MenuLayer;
class PauseMenu;
class PlayScene;
class RadialGradient;
	//magic
class FirePattern;
struct firepattern_properties;
class MagicEffect;
class MagicEffectDescriptor;
class MagicEffectSystem;
class Spell;
struct spell_cost;
class SpellDesc;
class SpellSystem;
	//misc
struct area_properties;
class AttributeSystem;
struct AreaStats;
struct ControlInfo;
class ControlListener;
class ControlRegister;
class ControlState;
class GSpace;
class GState;
struct parametric_motion_state;
class physics_context;
class PhysicsImpl;
class Sensor;

namespace ai {
    class FSM;
	class Function;
	class StateMachine;
	class Thread;
}

namespace Lua {
	class Inst;
}

enum class Attribute : int;

enum class GraphicsLayer : int;

typedef map<Attribute, float> AttributeMap;

typedef pair<int, int> int_pair;
typedef pair<float, float> float_pair;
typedef pair<double, double> double_pair;
typedef pair<int,int> IntVec2;
typedef tuple<long, long, long> TimerTriplet;
typedef vector<SpaceVect> Path;

typedef unsigned int LightID;
typedef unsigned int SpriteID;
typedef unsigned int ObjectIDType;

typedef list<GObject*, local_allocator<GObject*>> object_list;

typedef function<void(void)> zero_arity_function;
typedef function<void(const vector<string>&)> InterfaceFunction;

typedef function<GObject*(GSpace*, ObjectIDType)> ObjectGeneratorType;
typedef function<FiniteTimeAction*()> ActionGeneratorType;
typedef function<local_shared_ptr<FirePattern>(Agent*)> FirePatternGeneratorType;
//

struct debug_info
{
    const char* func;
    const char* file;
    int line;
};

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

enum class DamageType
{
	bullet = 0,
	bomb,
	effectArea,
	touch,
	melee,

	none
};

extern const boost::bimap<DamageType, string> damageTypeNameMap;

enum class Element
{
	ice = 0,
	sun,
	darkness,
	poison,

	none
};

extern const boost::bimap<Element, string> elementNameMap;

struct DamageInfo
{
	static DamageInfo bomb(float mag);
	static DamageInfo bullet(float mag);
	static DamageInfo melee(float mag);

	DamageInfo();
	DamageInfo(float mag, DamageType type);
	DamageInfo(float mag, DamageType type, Element element, float knockback);

	getter(float, mag);
	getter(float, knockback);
	getter(Element, element);
	getter(DamageType, type);
	getter(bool, damageOverTime);

	setter(float, mag);
	setter(float, knockback);
	setter(Element, element);
	setter(DamageType, type);
	setter(bool, damageOverTime);

	float mag = 0.0f;
	float knockback = 0.0f;
	//It is initialized to Attribute::end;
	Element element;
	DamageType type;

	bool damageOverTime = false;

	DamageInfo operator*(float rhs);
	bool isExplosion();
	bool isValid();
};

struct effect_attributes
{
	inline effect_attributes() {}

	inline effect_attributes(
		float magnitude,
		float length
	) :	
		magnitude(magnitude),
		length(length)
	{}

	inline effect_attributes(
		float magnitude,
		float length,
		float radius,
		DamageType type
	) :
		magnitude(magnitude),
		length(length),
		radius(radius),
		type(type)
	{}

	float magnitude = 0.0f;
	float length = 0.0f;
	float radius = 0.0f;
	DamageType type = DamageType::none;

	getter(float, magnitude)
	getter(float, radius)
	getter(float, length)
	getter(DamageType, type)

	setter(float, magnitude)
	setter(float, length)
	setter(float, radius)
	setter(DamageType, type)
};

typedef pair<const MagicEffectDescriptor*, effect_attributes> effect_entry;

struct app_params
{
	float getScale() const;

	inline unsigned int getWidth() const { return width; }
	inline unsigned int getHeight() const { return height; }
	inline bool getFullscreen() const { return fullscreen; }
	inline bool getVsync() const { return vsync; }
	inline bool getShowTimers() const { return showTimers; }
	inline float getDifficultyScale() const { return difficultyScale; }
	inline bool getUnlockAll() const { return unlockAllEquips; }
	inline unsigned int getFPS() const { return framesPerSecond; }
	inline double getFrameInterval() const { return secondsPerFrame; }

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

struct sprite_properties {
	string filename;
	IntVec2 size;
	int dpi;
	float referenceSize = 0.0f;
	//only used for animation sequences
	float duration = 0.0;
	Color3B color = Color3B::BLACK;
};

typedef pair<string, IntVec2> MapEntry;

struct area_properties
{
	static shared_ptr<area_properties> singleMap(string name);
    static shared_ptr<area_properties> noMap(string name);
    
	area_properties();

	string sceneName;
	string next;
	vector<MapEntry> maps;
	Color4F ambientLight;
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
    agentSensor = 0x80,
    playerGrazeRadar = 0x100,
    item = 0x200,
    npc = 0x400,
	floorSegment = 0x800,
	bomb = 0x1000,

	all = 0x1FFF,

	canDamage = 0x2000,
	interactible = 0x4000,
};

extern const GType obstacles;

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

	template <typename T>
	struct unique_usertype_traits<local_shared_ptr<T>> {
		typedef T type;
		typedef local_shared_ptr<T> actual_type;
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
