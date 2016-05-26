//
//  Prefix.h
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef Prefix_h
#define Prefix_h

//External includes should also be copied to build specific PCH.

//C standard libraries
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <type_traits>
#include <vector>

//All external libraries used by this app.
#include "chipmunk.hpp"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "lua.hpp"
#include "LuaBridge.h"

//Boost modules
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/assign.hpp>
#include <boost/bind.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/set.hpp>

using namespace std;
USING_NS_CC;
using namespace cp;
using namespace luabridge;

//app modules that are often used.
#include "macros.h"
#include "types.h"
#include "util.h"
#include "multifunction.h"

#include "controls.h"
#include "LuaError.h"
#include "LuaConvert.h"
#include "LuaAPI.hpp"
#include "LuaShell.hpp"
#include "GState.hpp"
#include "App.h"
#include "Spell.hpp"
#include "GAnimation.hpp"
#include "GSpace.hpp"
#include "GObject.hpp"
#include "Graphics.h"
#include "Dialog.hpp"
#include "scenes.h"
#include "PlayScene.hpp"
#include "menu.h"

#endif /* Prefix_h */
