//
//  util.h
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#ifndef util_h
#define util_h

//Define assignment conversion between cocos and chipmunk vectors
Vec2 toCocos(const SpaceVect& rhs);
SpaceVect toChipmunk(const Vec2& rhs);
SpaceVect toChipmunk(const cocos2d::Size& rhs);
IntVec2 toIntVector(const cocos2d::Size& rhs);

float dirToPhysicsAngle(Direction d);
Direction toDirection(SpaceVect);

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

//http://stackoverflow.com/a/17622468/807433
struct enum_hash
{
    template <typename T>
    inline
    typename std::enable_if<std::is_enum<T>::value, std::size_t>::type
    operator ()(T const value) const
    {
        return static_cast<std::size_t>(value);
    }
};

//Wrapper to call a method of a derived type with a base this.
template<typename Base, typename Derived, void (Derived::*Method)(void)>
function<void(Base*)> wrapAsBaseMethod()
{
    return [](Base* obj) -> void {
        Derived* d = dynamic_cast<Derived*>(obj);
        if(!d)
            throw runtime_error(StringUtils::format("GObject::methodWrap, object is not of type %s.", typeid(Derived).name()));
        
        mem_fn(Method)(d);
    };
}

void convertToUnitSpace(ValueMap& arg);
cocos2d::Rect getUnitspaceRectangle(const ValueMap& tileMapObj);

#endif /* util_h */
