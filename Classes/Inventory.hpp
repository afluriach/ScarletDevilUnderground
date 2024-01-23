//
//  Inventory.hpp
//  Koumachika
//
//  Created by Toni on 1/22/24.
//

#ifndef Inventory_hpp
#define Inventory_hpp

template<typename T>
struct equip_set
{
    vector<T> items;
    int idx = -1;
    
    inline T getCrnt()
    {
        if(idx != -1)
            return items.at(idx);
        else
            return nullptr;
    }
    
    inline T next()
    {
        if(idx == -1)
            return nullptr;
            
        ++idx;
        if(idx >= items.size())
            idx = 0;
            
        return items.at(idx);
    }
    
    inline T prev()
    {
        if(idx == -1)
            return nullptr;
        
        --idx;
        if(idx < 0)
            idx = items.size() - 1;
        
        return items.at(idx);
    }
    
    inline void clear()
    {
        items.clear();
        idx = -1;
    }
    
    inline void add(T item)
    {
        items.push_back(item);
        if(idx == -1)
            idx = 0;
    }
    
    inline string getIcon()
    {
        if(idx == -1)
            return "";
        else
            return items.at(idx)->getIcon();
    }
    
    inline bool hasItems()
    {
        return items.size() > 0;
    }
};

class Inventory
{
public:
    equip_set<local_shared_ptr<FirePattern>> firePatterns;
    equip_set<const SpellDesc*> powerAttacks;
	equip_set<const SpellDesc*> spells;
};

#endif /* Inventory_hpp */
