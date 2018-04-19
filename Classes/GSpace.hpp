//
//  GSpace.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GSpace_hpp
#define GSpace_hpp

#include "types.h"

class GObject;

#define OBJS_FROM_ARB \
    GObject* a = static_cast<GObject*>(arb.getBodyA().getUserData()); \
    GObject* b = static_cast<GObject*>(arb.getBodyB().getUserData());


class GSpace
{
public:
	typedef pair<GObject*, GObject*> object_pair;
	typedef pair<GType, GType> collision_type;
	typedef pair<object_pair, collision_type> contact;

    static const set<GType> selfCollideTypes;

    static const bool logPhysics = false;

    GSpace(Layer* graphicsLayer);    
    ~GSpace();

    GObject* addObject(const ValueMap& obj);
    GObject* addObject(GObject*);
    
    void addObjects(const ValueVector& objs);
    
    template<typename T>
    inline void addObjects(const vector<T*>& objects){
        static_assert(
            is_base_of<GObject, T>(),
            "addObjects: not a GObject type"
        );

        BOOST_FOREACH(T* o, objects)
            addObject(o);
    }
    
    void processAdditions();
    
    bool isObstacle(IntVec2);
    vector<SpaceVect> pathToTile(IntVec2 begin, IntVec2 end);
    void addNavObstacle(const SpaceVect& center, const SpaceVect& boundingDimensions);
    
    inline GObject* getObject(const string& name){
        auto it = objByName.find(name);
        return it != objByName.end() ? it->second : nullptr;
    }
    template<typename T>
    inline T* getObject(const string& name){
        static_assert(
            is_base_of<GObject, T>(),
            "getObject: not a GObject type"
        );
    
        auto it = objByName.find(name);
        
        if(it == objByName.end()) return nullptr;

        T* result = dynamic_cast<T*>(it->second);
        
        if(!result)
            throw runtime_error(StringUtils::format("getObject: %s is not of type %s", name.c_str(), typeid(T).name()));
        
        return result;
    }
    inline GObject* getObject(unsigned int uuid){
        auto it = objByUUID.find(uuid);
        return it != objByUUID.end() ? it->second : nullptr;
    }
    
    inline vector<string> getObjectNames(){
        auto key = [](pair<string,GObject*> e){return e.first;};
        vector<string> names(objByName.size());
        transform(objByName.begin(), objByName.end(), names.begin(), key);
        return names;
    }
    
    void removeObject(const string& name);
    void removeObject(GObject* obj);
    void processRemovals();
    
    unordered_map<int,string> getUUIDNameMap();
    
    void update();
    
    shared_ptr<Body> createCircleBody(
        const SpaceVect& center,
        float radius,
        float mass,
        GType type,
        PhysicsLayers layers,
        bool sensor,
        GObject* obj
    );
    shared_ptr<Body> createRectangleBody(
        const SpaceVect& center,
        const SpaceVect& dim,
        float mass,
        GType type,
        PhysicsLayers layers,
        bool sensor,
        GObject* obj
    );
    
    inline boost::dynamic_bitset<>* getNavMask() const { return navMask;}
    inline IntVec2 getSize() const {return spaceSize;}
    
    void addWallBlock(SpaceVect ll,SpaceVect ur);
    
	void addPath(string name, Path p);
	Path* getPath(string name);

    float distanceFeeler(GObject* agent, SpaceVect feeler, GType gtype);
    float obstacleDistanceFeeler(GObject* agent, SpaceVect feeler);
    float wallDistanceFeeler(GObject* agent, SpaceVect feeler);
    
    bool feeler(GObject* agent, SpaceVect feeler, GType gtype);
    bool obstacleFeeler(GObject* agent, SpaceVect feeler);
    bool wallFeeler(GObject* agent, SpaceVect feeler);
    
    inline void setSize(int x, int y){
        spaceSize = IntVec2(x,y);
        if(navMask)
            delete navMask;
        navMask = new boost::dynamic_bitset<>(x*y);
    }
    
    inline int getObjectCount(){
        return objByUUID.size();
    }
    
    inline unsigned int getFrame(){
        return frame;
    }
    
private:
    Space space;

	boost::unordered_map<GObject*,list<contact>> currentContacts;
	boost::unordered_map<collision_type, int(GSpace::*)(GObject*, GObject*)> beginContactHandlers;
	boost::unordered_map<collision_type, int(GSpace::*)(GObject*, GObject*)> endContactHandlers;

	void addContact(contact c);
	void removeContact(contact c);

    //The graphics destination to use for all objects constructed in this space.
    Layer* graphicsLayer;
    
    unsigned int frame = 1;

    unordered_map<unsigned int, GObject*> objByUUID;
    unordered_map<string, GObject*> objByName;

	unordered_map<string, Path> paths;
    
    //Objects which have been queued for addition. Will be added at end of frame.
    vector<GObject*> toAdd;
    //Objects whose additions have been processsed last frame. Physics has been initialized but
    //init has not yet run; it will run at start of frame.
    vector<GObject*> addedLastFrame;

    //Objects which have been queued for removal. Will be removed at end of frame.
    vector<GObject*> toRemove;
    
    boost::dynamic_bitset<>* navMask = nullptr;
    IntVec2 spaceSize;
    
    inline void markObstacleTile(int x, int y){
        if(x >= 0 && x < spaceSize.first){
            if(y >= 0 && y < spaceSize.second){
                (*navMask)[y*spaceSize.first+x] = 1;
            }
        }
    }
    
    inline bool isObstacleTile(int x, int y){
        if(x >= 0 && x < spaceSize.first){
            if(y >= 0 && y < spaceSize.second){
                return (*navMask)[y*spaceSize.first+x];
            }
        }
        return false;
    }

	template<GType TypeA, GType TypeB>
	inline int beginContact(Arbiter arb, Space& space)
	{
		OBJS_FROM_ARB

		auto it = beginContactHandlers.find(collision_type(TypeA, TypeB));

		//No collide;
		if(it == beginContactHandlers.end())
			return 0;

		if (a && b && it->second) {
			int(GSpace::*begin_method)(GObject*, GObject*) = it->second;
			(this->*begin_method)(a, b);
			contact c = contact(
				object_pair(a,b),
				collision_type(TypeA,TypeB)
			);
			addContact(c);
		}

		return 1;
	}

	template<GType TypeA, GType TypeB>
	inline int endContact(Arbiter arb, Space& space)
	{
		OBJS_FROM_ARB

		auto it = endContactHandlers.find(collision_type(TypeA, TypeB));

		//No collide
		if (it == endContactHandlers.end())
			return 0;

		if (a && b && it->second) {
			int(GSpace::*end_method)(GObject*, GObject*) = it->second;
			(this->*end_method)(a, b);
			contact c = contact(
				object_pair(a, b),
				collision_type(TypeA, TypeB)
			);
			removeContact(c);
		}

		return 1;
	}

	template<GType TypeA, GType TypeB>
	inline void AddHandler(int(GSpace::*begin)(GObject*, GObject*), int(GSpace::*end)(GObject*, GObject*))
	{
		space.addCollisionHandler(
			static_cast<CollisionType>(TypeA), 
			static_cast<CollisionType>(TypeB), 
			bind(&GSpace::beginContact<TypeA, TypeB>, this, placeholders::_1, placeholders::_2), 
			nullptr, 
			nullptr, 
			bind(&GSpace::endContact<TypeA, TypeB>, this, placeholders::_1, placeholders::_2) 
		); 

		beginContactHandlers[collision_type(TypeA, TypeB)] = begin;
		endContactHandlers[collision_type(TypeA,TypeB)] = end;
	}

    
    void initObjects();
    void addCollisionHandlers();
    
    void processRemoval(GObject* obj);    
	void processRemovalEndContact(GObject* obj);

	int playerEnemyBegin(GObject* a, GObject* b);
	int playerEnemyEnd(GObject* a, GObject* b);
	int playerEnemyBulletBegin(GObject* playerObj, GObject* bullet);
	int playerBulletEnemyBegin(GObject* a, GObject* b);
	int playerFlowerBegin(GObject* a, GObject* b);
    int playerCollectibleBegin(GObject* a, GObject* b);
	int bulletEnvironment(GObject* a, GObject* b);
	int noCollide(GObject* a, GObject* b);
	int bulletWall(GObject* bullet, GObject* unused);
	int sensorStart(GObject* radarAgent, GObject* target);
	int sensorEnd(GObject* radarAgent, GObject* target);
};

#endif /* GSpace_hpp */
