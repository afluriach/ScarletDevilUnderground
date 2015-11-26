//
//  Prefix.h
//  FlansBasement
//
//  Created by Toni on 11/25/15.
//
//

#ifndef Prefix_h
#define Prefix_h

//C standard libraries
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <set>
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
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/math/constants/constants.hpp>

using namespace std;
USING_NS_CC;

//app modules that are often used.
#include "macros.h"
#include "types.h"
#include "util.h"
#include "multifunction.h"

#include "controls.h"
#include "LuaAPI.hpp"
#include "LuaShell.hpp"
#include "App.h"
#include "GAnimation.hpp"
#include "GSpace.hpp"
#include "GObject.hpp"
#include "scenes.h"
#include "Graphics.h"
#include "menu.h"

#endif /* Prefix_h */