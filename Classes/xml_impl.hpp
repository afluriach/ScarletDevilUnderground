//
//  xml_impl.hpp
//  Koumachika
//
//  Created by Toni on 4/23/23.
//
//

#ifndef xml_impl_hpp
#define xml_impl_hpp

namespace app {

    typedef function< bool(tinyxml2::XMLElement*, MagicEffectDescriptor**)> effect_parser;
    typedef function< bool(tinyxml2::XMLElement*, SpellDesc**)> spell_parser;

    extern const unordered_map<string, spell_parser> spellParsers;
    extern const unordered_map<string, effect_parser> effectParsers;

    GObject::AdapterType objectAdapter(local_shared_ptr<agent_properties> props); //NO-OP
    GObject::AdapterType objectAdapter(local_shared_ptr<bomb_properties> props); //NO-OP
    GObject::AdapterType objectAdapter(local_shared_ptr<bullet_properties> props); //NO-OP
    GObject::AdapterType objectAdapter(local_shared_ptr<effectarea_properties> props);
    GObject::AdapterType objectAdapter(local_shared_ptr<enemy_properties> props);
    GObject::AdapterType objectAdapter(local_shared_ptr<npc_properties> props);
    GObject::AdapterType objectAdapter(local_shared_ptr<item_properties> props);
    GObject::AdapterType objectAdapter(local_shared_ptr<floorsegment_properties> props);
    GObject::AdapterType objectAdapter(local_shared_ptr<environment_object_properties> props);

    bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<object_properties> result);
    bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<environment_object_properties> result);
    bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<effectarea_properties> result);
    bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<agent_properties> result);
    bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<area_properties>* result);
    bool parseObject(tinyxml2::XMLElement* elem, AttributeMap* result);
    bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<enemy_properties> result);
    bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<npc_properties> result);
    bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<firepattern_properties>* result);
    bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<floorsegment_properties> result);
    bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<sprite_properties>* result);
    bool parseObject(tinyxml2::XMLElement* elem, shared_ptr<LightArea>* result);
    bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<bullet_properties> result);
    bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<bomb_properties> result);
    bool parseObject(tinyxml2::XMLElement* elem, MagicEffectDescriptor** result);
    bool parseObject(tinyxml2::XMLElement* elem, local_shared_ptr<item_properties> result);
    bool parseObject(tinyxml2::XMLElement * elem, SpellDesc** result);

    bool autoName(tinyxml2::XMLElement* elem, string& field);

    bool getAttributeAttr(tinyxml2::XMLElement* elem, const string& name, Attribute* result);
    bool getElementAttr(tinyxml2::XMLElement* elem, const string& name, Element* result);
    bool getStringAttr(tinyxml2::XMLElement* elem, const string& name, string* result);
    bool getColorAttr(tinyxml2::XMLElement* elem, const string& name, Color4F* result);
    bool getVector(tinyxml2::XMLElement* elem, const string& name, SpaceVect* result);
    bool getDamageInfo(tinyxml2::XMLElement* elem, DamageInfo* result);

    void logXmlError(tinyxml2::XMLDocument* doc, string filename);

    template<typename T>
    inline local_shared_ptr<T> getObjectProperties(const string& name)
    {
        local_shared_ptr<object_properties> obj = getOrDefault(app::objects, name);

        if (!obj) {
            log_print("%s not found!", name);
            return nullptr;
        }

        local_shared_ptr<T> t = obj.downcast<T>();

        if (!t) {
            log_print("%s is not of type %s!", name, typeid(T).name());
            return nullptr;
        }

        return t;
    }

    template<typename T>
    bool copyBaseObject(
        tinyxml2::XMLElement* elem,
        unordered_map<string, local_shared_ptr<T>>& _map,
        T* output
    ) {
        string base;
        if (getStringAttr(elem, "base", &base)) {
            auto _base = getOrDefault(_map, base);
            if (_base) {
                *output = *_base;
                return true;
            }
            else {
                log_print("%s: unknown base %s type: %s", elem->Name(), typeid(T).name(), base);
            }
        }
        return false;
    }

    template<typename T>
    local_shared_ptr<T> copyBaseObjectShared(const string& baseName) {
        local_shared_ptr<T> _base = getObjectProperties<T>(baseName);
        if (_base) {
            local_shared_ptr<T> result = make_local_shared<T>();
            *result = *_base;
            return result;
        }
        else {
            return nullptr;
        }
    }

    template<typename T>
    bool copyBaseObjectShared(
        tinyxml2::XMLElement * elem,
        local_shared_ptr<T> output
    ) {
        string base;
        if (getStringAttr(elem, "base", &base)) {
            local_shared_ptr<T> _base = getObjectProperties<T>(base);
            if (_base) {
                *output = *_base;
                return true;

            }
            else {
                log_print("%s: unknown base %s type: %s", elem->Name(), typeid(T).name(), base);

            }

        }
        return false;

    }

    template<typename T>
    inline void loadObjects(string filename, unordered_map<string,T>& _map)
    {
        tinyxml2::XMLDocument objects;
        auto error = objects.Parse(io::loadTextFile(filename).c_str());

        if (error != tinyxml2::XML_SUCCESS) {
            logXmlError(&objects, filename);
            return;
        }

        tinyxml2::XMLNode* root = objects.FirstChild();

        for (
            tinyxml2::XMLElement* crnt = root->FirstChildElement();
            crnt != nullptr;
            crnt = crnt->NextSiblingElement() )
        {
            T object;
            if (parseObject(crnt, &object)) {
                _map.insert_or_assign(crnt->Name(), object);
            }
            else {
                log_print("%s : %s failed to load!", filename, crnt->Name());
            }
        }
    }

    template<typename T>
    inline void loadObjectsShared(string filename)
    {
        tinyxml2::XMLDocument objectsDocument;
        auto error = objectsDocument.Parse(io::loadTextFile(filename).c_str());

        if (error != tinyxml2::XML_SUCCESS) {
            logXmlError(&objectsDocument, filename);
            return;
        }

        tinyxml2::XMLNode* root = objectsDocument.FirstChild();

        for (
            tinyxml2::XMLElement* crnt = root->FirstChildElement();
            crnt != nullptr;
            crnt = crnt->NextSiblingElement())
        {
            local_shared_ptr<T> object = make_local_shared<T>();
            copyBaseObjectShared<T>(crnt, object);

            if (parseObject(crnt, object)) {
                app::objects.insert_or_assign(crnt->Name(), object);

                auto adapter = objectAdapter(object);
                if (adapter) {
                    GObject::namedObjectTypes.insert_or_assign(crnt->Name(),adapter);
                }
            }
            else {
                log_print("%s : %s failed to load!", filename, crnt->Name());
            }
        }
    }

    template<typename T>
    bool getSubObject(
        tinyxml2::XMLElement* elem,
        string fieldName,
        local_shared_ptr<T>* result,
        bool autoName
    ) {
        string field;
        getStringAttr(elem, fieldName, &field);

        if (autoName && field == "auto") {
            field = elem->Name();
        }

        auto _result = getObjectProperties<T>(field);
        if (_result) {
            *result = _result;
        }

        return result;
    }

    template<typename T>
    bool getSubObject(
        tinyxml2::XMLElement* elem,
        string fieldName,
        shared_ptr<T>* result,
        const unordered_map<string, shared_ptr<T>>& _map,
        bool autoName
    ) {
        string field;
        getStringAttr(elem, fieldName, &field);

        if (autoName && field == "auto") {
            field = elem->Name();
        }

        auto it = _map.find(field);

        if (it != _map.end()) {
            *result = it->second;
        }

        return it != _map.end();
    }

    template<typename T>
    bool getSubObjectPtr(
        tinyxml2::XMLElement* elem,
        string fieldName,
        const T** result,
        const unordered_map<string, T*>& _map,
        bool autoName
    ) {
        string field;
        getStringAttr(elem, fieldName, &field);

        if (autoName && field == "auto") {
            field = elem->Name();
        }

        auto it = _map.find(field);

        if (it != _map.end()) {
            *result = it->second;
        }

        return it != _map.end();
    }

    template<typename T>
    bool getNumericAttr(tinyxml2::XMLElement* elem, const string& name, T* result)
    {
        const char* attr = elem->Attribute(name.c_str());

        if (attr) {
            try {
                *result = boost::lexical_cast<T, string>(attr);
                return true;
            }
            catch (boost::bad_lexical_cast ex) {
                log_print("Unable to parse XML attribute %s", name);
            }
        }
        return false;
    }
}

#endif 
