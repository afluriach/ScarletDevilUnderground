//
//  GSpace.hpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GSpace_hpp
#define GSpace_hpp

class GObject;

//GSpace.hpp is already included in PCH

class GSpace
{
public:
    //Type signifies the collision handler as well as the group, but the group can be overidden:
    //Normally objects only collide with those of non-matching group. If we want an object to collide
    //with everything, use 0 for its group instead of its (non-zero) type.
    static const set<GType> selfCollideTypes;

    static const bool logPhysics = false;

    GSpace(Layer* graphicsLayer);    
    ~GSpace();

    GObject* addObject(const ValueMap& obj);
    GObject* addObject(GObject*);
    void addObjects(const ValueVector& objs);
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
    
    float distanceFeeler(GObject* agent, SpaceVect feeler, GType gtype);
    float obstacleFeeler(GObject* agent, SpaceVect feeler);
    float wallFeeler(GObject* agent, SpaceVect feeler);
    
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
    //The graphics destination to use for all objects constructed in this space.
    Layer* graphicsLayer;
    
    unsigned int frame = 1;

    unordered_map<unsigned int, GObject*> objByUUID;
    unordered_map<string, GObject*> objByName;
    
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
    
    void initObjects();
    void addCollisionHandlers();
    
    void processRemoval(GObject* obj);    
};

#endif /* GSpace_hpp */
