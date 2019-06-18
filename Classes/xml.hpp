//
//  xml.hpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#ifndef xml_hpp
#define xml_hpp

class LightArea;

namespace app {
	extern unordered_map<string, floorsegment_properties> floors;
	extern unordered_map<string, shared_ptr<LightArea>> lights;

	void loadFloors();
	void loadLights();

	shared_ptr<LightArea> getLight(const string& name);

	template<typename T>
	inline void loadObjects(string filename, unordered_map<string,T>& _map)
	{
		tinyxml2::XMLDocument objects;
		auto error = objects.Parse(io::loadTextFile(filename).c_str());

		if (error != tinyxml2::XML_SUCCESS) {
			log("XML error: %d", error);
			return;
		}

		tinyxml2::XMLNode* root = objects.FirstChild();

		for (
			tinyxml2::XMLElement* crnt = root->FirstChildElement();
			crnt != nullptr;
			crnt = crnt->NextSiblingElement() )
		{
			T object;
			if(parseObject(crnt, &object))
				_map.insert_or_assign(crnt->Name(), object);
		}
	}

	bool parseObject(tinyxml2::XMLElement* elem, floorsegment_properties* result);
	bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<LightArea>* result);
}

#endif 
