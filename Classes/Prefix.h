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

#ifdef CC_TARGET_OS_MAC
    #import <OpenAL/al.h>
    #import <OpenAL/alc.h>
    #define ALuint uint32_t
    #define use_gamepad 1
#endif

#ifdef CC_TARGET_OS_IPHONE
    #import <OpenAL/al.h>
    #import <OpenAL/alc.h>
    #define ALuint uint32_t
	#define use_gamepad 0
#endif

#if defined _WIN32
    #include "AL/al.h"
    #include "AL/alc.h"
    #define use_gamepad 1
#endif

//C standard libraries
#include <algorithm>
#include <any>
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
#include <queue>
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
#include "Box2D/Box2D.h"
#include "cocos2d.h"
#include "tinyxml2/tinyxml2.h"
#include "ui/CocosGUI.h"
#include "lua.hpp"
#include "sol/sol.hpp"
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
#include <boost/pool/pool_alloc.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/irange.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>

using namespace std;
USING_NS_CC;

typedef double SpaceFloat;

#include "PerlinNoise.hpp"
#include "allocator.hpp"
#include "shared_ptr.hpp"
#include "macros.h"
#include "enum.h"
#include "vect.hpp"
#include "types.h"
#include "graphics_types.h"
#include "util.h"
#include "log.hpp"
#include "object_ref.hpp"
#include "object_params.hpp"
#include "controls.h"
#include "GObject.hpp"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "FileIO.hpp"
#include "xml.hpp"
#include "Attributes.hpp"
#include "App.h"
#include "GState.hpp"

#define DEV_MODE 1
#define GSPACE_MULTITHREAD 1
#define INTERNAL_PREFIX 0

#if INTERNAL_PREFIX
#include "internal_prefix.h"
#endif

#endif /* Prefix_h */
