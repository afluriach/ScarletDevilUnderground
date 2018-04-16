//
//  GObject.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GObject_hpp
#define GObject_hpp

#include "multifunction.h"
#include "types.h"

class Player;
class Bullet;
class GSpace;
namespace Lua{class Inst;}
class Spell;

class GObject
{
public:
    typedef function<GObject*( const ValueMap&) > AdapterType;
    
    static constexpr bool logCreateObjects = false;
	//Map each class name to a constructor adapter function.
	static const unordered_map<string, AdapterType> adapters;
	static unsigned int nextUUID;

	static GObject* constructByType(const string& type, const ValueMap& args);
	
	//Representation as a map object
    GObject(const ValueMap& args);
    GObject(const string& name, const SpaceVect& pos);
    
    virtual ~GObject();

	//object identification, init, and update
	const string name;
	const unsigned int uuid;

	util::multifunction<void(void)> multiInit;
	util::multifunction<void(void)> multiUpdate;

	inline string getName() {
		return name;
	}

	inline unsigned int getUUID() {
		return uuid;
	}

	inline static void resetObjectUUIDs() {
		nextUUID = 1;
	}

	//Called on the first frame after it has been added, before update is called on it or any other
	//objects in the same frame
	inline void init(){
		multiInit();
        setupLuaContext();
		runLuaInit();
	}

	inline void update(){
		multiUpdate();
		runLuaUpdate();
	}
    
	//BEGIN PHYSICS

	shared_ptr<Body> body;
	shared_ptr<Body> radar;

    //Posiition where the object was loaded
    SpaceVect initialCenter;

    void setInitialVelocity(const SpaceVect&& v);
    Vec2 getInitialCenterPix();

    SpaceVect getPos();
    void setPos(float x, float y);

    virtual void setAngle(float a);
    float getAngle();
    void rotate(float a);
    SpaceVect getFacingVector();
    virtual void setDirection(Direction d);
    SpaceVect getVel();
    void setVel(SpaceVect v);
    //Apply a force as impulse where t = frame length.
    void applyForceForSingleFrame(SpaceVect f);
    void applyImpulse(float mag, float angle);

	//A default of 0 signifies undefined. Using -1 to indicate static or positive for dynamic.
	virtual float getMass() const = 0;
	virtual GType getType() const = 0;
	virtual inline bool getSensor() const { return false; }
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::ground; }

	inline virtual float getMaxSpeed() const { return 0; }
	inline virtual float getMaxAcceleration() const { return 0; }

	//Called before adding the the object to space.
	virtual void initializeBody(GSpace& space) = 0;
	inline virtual void initializeRadar(GSpace& space) {};

	//Create Node which graphically reprensets this object and adds it to Layer
	virtual void initializeGraphics(Layer* layer) = 0;

	//END PHYSICS
    
	//BEGIN LUA
	
	unique_ptr<Lua::Inst> ctx;

	inline virtual string getScriptName() const { return ""; }

    string getScriptVal(string name);
    void setScriptVal(string field, string val);
    string _callScriptVal(string field, string args);
    void runLuaInit();
    void runLuaUpdate();
	void setupLuaContext();

	//END LUA    
};

#endif /* GObject_hpp */
