//
//  GSpace.hpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GSpace_hpp
#define GSpace_hpp

#include <map>
#include <memory>
#include <vector>

#include "cocos2d.h"
#include "chipmunk.hpp"

#include "GObject.hpp"

using namespace std;
USING_NS_CC;

class GSpace
{
public:
    GSpace();

    void addObject(const ValueMap& obj);
    void addObjects(const ValueVector& objs);
    void processAdditions(cocos2d::Layer* graphicsLayer);
    
    static std::shared_ptr<cp::Body> createRectangleBody(
        cp::Space& space,
        const cp::Vect& center,
        const cp::Vect& dim,
        float mass,
        GObject* obj
    );
private:
    cp::Space space;
    
    map<string, GObject*> objByName;
    map<string, vector<GObject*>> objsByType;
    
    vector<GObject*> toAdd;
};

#endif /* GSpace_hpp */