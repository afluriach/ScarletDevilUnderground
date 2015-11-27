//
//  util.h
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef util_h
#define util_h

//Define assignment conversion between cocos and chipmunk vectors
cocos2d::Vec2 toCocos(const cp::Vect& rhs);
cp::Vect toChipmunk(const cocos2d::Vec2& rhs);
IntVec2 toIntVector(const cocos2d::Size& rhs);

float dirToPhysicsAngle(Direction d);

cocos2d::Scene* crntScene();

void printValueMap(const cocos2d::ValueMap& obj);
float getFloat(const cocos2d::ValueMap& obj, const std::string& name);

cocos2d::Sprite* loadImageSprite(const std::string& resPath, GraphicsLayer sceneLayer, cocos2d::Layer* dest, const Vec2& pos);

inline std::string getRealPath(const std::string& path)
{
    return FileUtils::getInstance()->fullPathForFilename(path);
}

inline std::string loadTextFile(const std::string& res)
{
    return cocos2d::FileUtils::getInstance()->getStringFromFile(res);
}

inline std::vector<std::string> splitString(const std::string& input,const std::string& sep)
{
    std::vector<std::string> output;
    boost::split(output, input,boost::is_any_of(sep));
    return output;
}

//Bind this but not the explicit arguments.
template<typename T, typename A1>
function<void(A1)> bindMethod(void (T::* m)(A1), T* This)
{
    return bind(m, This, placeholders::_1);
}

template<typename T, typename A1, typename A2>
function<void(A1,A2)> bindMethod(void (T::* m)(A1,A2), T* This)
{
    return bind(m, This, placeholders::_1, placeholders::_2);
}

#endif /* util_h */
