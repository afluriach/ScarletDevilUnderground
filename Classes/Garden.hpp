//
//  Garden.hpp
//  Koumachika
//
//  Created by Toni on 12/1/15.
//
//

#ifndef Garden_hpp
#define Garden_hpp

class GardenBlooming : public PlayScene
{
public:
    CREATE_FUNC(GardenBlooming);
    inline GardenBlooming() : PlayScene("GardenBlooming"){
        multiInit += bind(&GardenBlooming::generateFlowers, this);
    }
    
    static const vector<string> colors;

    static constexpr float weight = 0.3;
    void generateFlowers();
    void addFlower(const SpaceVect& pos);
};

#endif /* Garden_hpp */
