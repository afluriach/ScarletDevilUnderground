//
//  GState.hpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#ifndef GState_hpp
#define GState_hpp

//All of the persistent state associated with a single profile.
class GState
{
public:
    static const unsigned int version = 1;
    
    friend class boost::serialization::access;

    set<string> itemRegistry;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & itemRegistry;
    }
};

#endif /* GState_hpp */
