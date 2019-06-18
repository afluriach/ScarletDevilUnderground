//
//  xml.cpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#include "Prefix.h"

#include "FileIO.hpp"
#include "xml.hpp"

namespace app {

unordered_map<string, floorsegment_properties> floors;

void loadFloors()
{
	loadObjects<floorsegment_properties>("objects/floors.xml", app::floors);
}

bool parseObject(tinyxml2::XMLElement* elem, floorsegment_properties* result)
{
	string sfx;
	double traction = 1.0;

	if (auto attr = elem->Attribute("sfx"))
		sfx = string(attr);

	if (auto attr = elem->Attribute("traction"))
		traction = elem->DoubleAttribute("traction");

	*result = floorsegment_properties{ sfx, traction };
	return true;
}

}
