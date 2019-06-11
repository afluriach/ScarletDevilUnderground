//
//  Prefix.h
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef Prefix_h
#define Prefix_h

#ifdef _WIN32
//"Inherited via dominance" warning
#pragma warning(disable : 4250)
#endif

#if defined(TARGET_OS_OSX) && defined(__OBJC__)
	#import <Cocoa/Cocoa.h>
#endif

#if defined(TARGET_OS_IOS) && defined(__OBJC__)
    #import <Foundation/Foundation.h>
    #import <UIKit/UIKit.h>
#endif

#if defined(TARGET_OS_IOS)
    #define use_gamepad 0
#else
    #define use_gamepad 1
#endif

//C standard libraries
#include <algorithm>
#include <atomic>
#include <bitset>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_set>
#include <utility>
#include <vector>

//All external libraries used by this app.
#include "AL/al.h"
#include "AL/alc.h"
#include "chipmunk.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "lua.hpp"
#include "LuaBridge.h"
#include "noise.h"
#include "sndfile.h"

#if use_gamepad
    #include "gainput/gainput.h"
#endif

#if defined(TARGET_OS_IOS) && defined(__OBJC__)
    #import "platform/ios/CCEAGLView-ios.h"
#endif

//Boost modules
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/icl/interval_map.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/irange.hpp>
#include <boost/rational.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>

using namespace std;
USING_NS_CC;
using namespace luabridge;

#include "macros.h"
#include "vect.hpp"
#include "types.h"

template<typename... T>
void log_print(string s, T... args)
{
	boost::format fmt(s);
	string result = boost::str((fmt % ... % forward<T>(args)));

	Director::getInstance()->logOutput(result);
}

#define log log_print

#define DEV_MODE 1

#endif /* Prefix_h */
