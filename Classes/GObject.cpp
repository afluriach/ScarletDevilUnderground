//
//  GObject.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "GObject.hpp"
#include "util.h"

USING_NS_CC;

GObject::GObject(const ValueMap& obj)
{
    log("GObject instantiated.");
    printValueMap(obj);
}