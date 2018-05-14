//
//  util.h
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#ifndef util_h
#define util_h

#include "types.h"

//Define assignment conversion between cocos and chipmunk vectors
Vec2 toCocos(const SpaceVect& rhs);
SpaceVect toChipmunk(const Vec2& rhs);
SpaceVect toChipmunk(const cocos2d::CCSize& rhs);
IntVec2 toIntVector(const cocos2d::CCSize& rhs);

float dirToPhysicsAngle(Direction d);
SpaceVect dirToVector(Direction d);
//Cannot be overloaded since it is exposed in the Lua API.
Direction toDirection(SpaceVect);
Direction angleToDirection(float a);
Direction stringToDirection(string str);
float canonicalAngle(float a);

float toDegrees(float a);

float circleMomentOfInertia(float mass, float radius);
float rectangleMomentOfInertia(float mass, const SpaceVect& dim);

Scene* crntScene();

void printValueMap(const ValueMap& obj);
float getFloat(const ValueMap& obj, const string& name);
float getFloatOrDefault(const ValueMap& obj, const string& name, float def);

Sprite* loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest, const Vec2& pos, float zoom);

Label* createTextLabel(const string& text, int size);

string getRealPath(const string& path);
string loadTextFile(const string& res);
vector<string> splitString(const string& input,const string& sep);

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

//call function using tuple
namespace detail
{
    template <typename Ret, typename F, typename Tuple, bool Done, int Total, int... N>
    struct call_impl
    {
        static Ret call(F f, Tuple && t)
        {
            return call_impl<Ret, F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
        }
    };

    template <typename Ret, typename F, typename Tuple, int Total, int... N>
    struct call_impl<Ret, F, Tuple, true, Total, N...>
    {
        static Ret call(F f, Tuple && t)
        {
            return f(std::get<N>(std::forward<Tuple>(t))...);
        }
    };
}
template <typename Ret, typename F, typename Tuple>
Ret variadic_call(F f, Tuple && t)
{
    typedef typename std::decay<Tuple>::type ttype;
    return detail::call_impl<Ret, F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call(f, std::forward<Tuple>(t));
}

#define wrap_method(cls,method,This) wrapMethod<cls,&cls::method>(This)

//Wrapper to call a method of a derived type with a base this.
template<typename Class, void (Class::*Method)(void)>
function<void()> wrapMethod(Class* This)
{
    return [=]() -> void {
        mem_fn(Method)(This);
    };
}

template<typename T>
T vmin(T&&t)
{
  return std::forward<T>(t);
}

template<typename T0, typename T1, typename... Ts>
typename std::common_type<
  T0, T1, Ts...
>::type vmin(T0&& val1, T1&& val2, Ts&&... vs)
{
  if (val2 < val1)
    return vmin(val2, std::forward<Ts>(vs)...);
  else
    return vmin(val1, std::forward<Ts>(vs)...);
}

void convertToUnitSpace(ValueMap& arg);
cocos2d::CCRect getUnitspaceRectangle(const ValueMap& tileMapObj);

void checkCreateProfileSubfolder();
string getProfilePath();

#endif /* util_h */
