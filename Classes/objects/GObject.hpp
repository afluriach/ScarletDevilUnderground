//
//  GObject.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GObject_hpp
#define GObject_hpp

#include "enum.h"
#include "multifunction.h"
#include "object_ref.hpp"
#include "types.h"

class Player;
class Bullet;
class GSpace;
namespace Lua{class Inst;}
class SpaceLayer;
class Spell;
class MagicEffect;
class FloorSegment;

#define MapObjCons(cls) cls(GSpace* space, ObjectIDType id, const ValueMap& args)
#define MapObjForwarding(cls) cls(space,id,args)

class GObject
{
public:
	typedef function<GObject*(GSpace*, ObjectIDType, const ValueMap&) > AdapterType;
	typedef function<GObject*(GSpace*, ObjectIDType)> GeneratorType;

    static constexpr bool logCreateObjects = false;
	//Map each class name to a constructor adapter function.
	static const unordered_map<string, AdapterType> adapters;

	static GObject* constructByType(GSpace* space, ObjectIDType id, const string& type, const ValueMap& args);
	static GeneratorType factoryMethodByType(const string& type, const ValueMap& args);

	template<class ObjectCls, typename... ConsArgs>
	static inline GObject* create(GSpace* space, ObjectIDType id, ConsArgs...args)
	{
		return new ObjectCls(space,id,args...);
	}

	template<class ObjectCls, typename... ConsArgs>
	static inline GeneratorType make_object_factory(ConsArgs... args)
	{
		return [args...](GSpace* space, ObjectIDType id) -> GObject* {
			return create<ObjectCls, ConsArgs...>(space, id, args...);
		};
	}

    template<typename T>
    inline bool isType(){
        return dynamic_cast<T*>(this) != nullptr;
    }	

	//Representation as a map object
    GObject(GSpace* space, ObjectIDType uuid, const ValueMap& args);
	GObject(GSpace* space, ObjectIDType uuid, const ValueMap& args, bool anonymous);
	GObject(GSpace* space, ObjectIDType uuid, const string& name, const SpaceVect& pos, bool anonymous);
	GObject(GSpace* space, ObjectIDType uuid, const string& name, const SpaceVect& pos, SpaceFloat angle, bool anonymous);
    
    virtual ~GObject();

	//object identification, init, and update
	const string name;
    const bool anonymous = false;
	const ObjectIDType uuid;
	GSpace *const space;

	util::multifunction<void(void)> multiInit;
	util::multifunction<void(void)> multiUpdate;

	inline string getName() const {
		return name;
	}

	inline ObjectIDType getUUID() const {
		return uuid;
	}

	//Called on the first frame after it has been added, before update is called on it or any other
	//objects in the same frame
	void init();
	void update();
    
	//BEGIN PHYSICS

	shared_ptr<Body> body;
	shared_ptr<Body> radar;

    //Posiition where the object was loaded
    SpaceVect initialCenter;
	//Tracks setting of physics properties, they will be applied before physics step.
	PhysicsProperties physicsPropertiesToApply;

	object_ref<FloorSegment> crntFloor;
	
    void setInitialVelocity(const SpaceVect&& v);
    void setInitialAngle(SpaceFloat a);
    void setInitialAngularVelocity(SpaceFloat w);
    Vec2 getInitialCenterPix();

	void applyPhysicsProperties();
	void updateFloorSegment();

    SpaceVect getPos() const;
    void setPos(SpaceFloat x, SpaceFloat y);

    virtual void setAngle(SpaceFloat a);
	SpaceFloat getAngle() const;
    
    void rotate(SpaceFloat a);
    SpaceVect getFacingVector() const;
    virtual void setDirection(Direction d);
    
    SpaceVect getVel() const;
    void setVel(SpaceVect v);
    
	SpaceFloat getAngularVel()const;
    void setAngularVel(SpaceFloat w);
    
    //Apply a force as impulse where t = frame length.
    void applyForceForSingleFrame(SpaceVect f);
    void applyImpulse(SpaceFloat mag, SpaceFloat angle);

	PhysicsLayers getCrntLayers();
	void setLayers(PhysicsLayers layers);

	//A default of 0 signifies undefined. Using -1 to indicate static or positive for dynamic.
	virtual SpaceFloat getMass() const = 0;
	virtual SpaceFloat getRadius() const = 0;
    virtual SpaceFloat getMomentOfInertia() const = 0;
	virtual GType getType() const = 0;
	virtual inline bool getSensor() const { return false; }
	virtual inline PhysicsLayers getLayers() const { return enum_bitwise_or(PhysicsLayers,floor,ground) ; }

	inline virtual SpaceFloat getMaxSpeed() const { return 0; }
	inline virtual SpaceFloat getMaxAcceleration() const { return 0; }

	//Called before adding the the object to space.
	virtual void initializeBody(GSpace& space) = 0;
	inline virtual void initializeRadar(GSpace& space) {};

	void updateRadarPos();

	//END PHYSICS
    
	//BEGIN LUA
	
    #define if_lua_ctx if(!anonymous && ctx) 
    
	unique_ptr<Lua::Inst> ctx;

	inline virtual string getScriptName() const { return ""; }

    string getScriptVal(string name);
    void setScriptVal(string field, string val);
    string _callScriptVal(string field, string args);
    void runLuaInit();
    void runLuaUpdate();
	void setupLuaContext();

	//END LUA
    
    //BEGIN GRAPHICS
    
    Node* sprite = nullptr;
    
        //The Z-order used by Cocos2D.
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    inline int sceneLayerAsInt(){return static_cast<int>(sceneLayer());};
    inline virtual float zoom() const {return 1.0f;}
    void updateSprite();

	//Create Node which graphically reprensets this object and adds it to Layer
	virtual void initializeGraphics(SpaceLayer* layer) = 0;
	
	inline virtual void setSpriteShader(const string& shaderName){
        log("GObject::setSpriteShader: virtual base, no implementation for %s!", name.c_str());
    }

    //END GRAPHICS

	//BEGIN SPELLS

	void cast(unique_ptr<Spell> spell);
	void cast(const string& name, const ValueMap& args);
	//For Luabridge
	void castByName(string name, const ValueMap& args);

	void stopSpell();
	void updateSpells();

	inline bool isSpellActive() const {
		return static_cast<bool>(crntSpell);
	}

	unique_ptr<Spell> crntSpell;

	list<shared_ptr<MagicEffect>> magicEffects;
	list<shared_ptr<MagicEffect>> magicEffectsToAdd;
	list<shared_ptr<MagicEffect>> magicEffectsToRemove;

	virtual void addMagicEffect(shared_ptr<MagicEffect> effect);
	void updateMagicEffects();

	template<class T>
	inline bool hasMagicEffect()
	{
		for (auto it = magicEffects.begin(); it != magicEffects.end(); ++it) {
			if (dynamic_cast<T*>(it->get())) {
				return true;
			}
		}
		return false;
	}

	//END SPELLS

	//BEGIN SENSORY

	bool isInvisible = false;

	inline bool getInvisible() { return isInvisible; }
	inline void setInvisible(bool val) { isInvisible = val; }
	
	//END SENSORY
};

#endif /* GObject_hpp */
