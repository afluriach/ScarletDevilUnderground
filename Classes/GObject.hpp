//
//  GObject.hpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#ifndef GObject_hpp
#define GObject_hpp

#include <map>

#include "cocos2d.h"

class GObject
{
public:
    typedef std::function<GObject*( const cocos2d::ValueMap&) > AdapterType;
    
    //Representation as a map object
    GObject(const cocos2d::ValueMap& args);
    
    template <typename T>
    static AdapterType consAdapter()
    {
        return [](const cocos2d::ValueMap& args) -> GObject* { return new T(args); };
    }
    
    //Map each class name to a constructor adapter function.
    static const std::map<std::string,AdapterType> adapters;
    
    static GObject* constructByType(const std::string& type, const cocos2d::ValueMap& args );
};

#endif /* GObject_hpp */
