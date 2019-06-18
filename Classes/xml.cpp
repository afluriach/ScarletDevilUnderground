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
	tinyxml2::XMLDocument floors;
	auto error = floors.Parse(io::loadTextFile("objects/floors.xml").c_str());

	if (error != tinyxml2::XML_SUCCESS) {
		log("XML error: %d", error);
		return;
	}

	tinyxml2::XMLNode* root = floors.FirstChild();

	for (
		tinyxml2::XMLElement* crnt = root->FirstChildElement();
		crnt != nullptr;
		crnt = crnt->NextSiblingElement()
		) {
		string sfx;
		double traction = 1.0;

		if (auto attr = crnt->Attribute("sfx"))
			sfx = string(attr);

		if (auto attr = crnt->Attribute("traction"))
			traction = crnt->DoubleAttribute("traction");

		app::floors.insert_or_assign(crnt->Name(), floorsegment_properties{ sfx, traction });
	}
}

}
