//
//  AppPrefix.h
//  FlansBasement
//
//  Created by Toni on 11/25/15.
//
//

#ifndef AppPrefix_h
#define AppPrefix_h

//This is used by the precompiled header AppPrefix.pch

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

//app modules that are often used.
#include "types.h"
#include "App.h"
#include "macros.h"
#include "util.h"

#endif /* AppPrefix_h */
