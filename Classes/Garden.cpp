//
//  Garden.cpp
//  Koumachika
//
//  Created by Toni on 12/1/15.
//
//

#include "Prefix.h"

#include "Flower.h"
#include "Garden.hpp"
#include "GSpace.hpp"

const vector<string> GardenBlooming::colors = boost::assign::list_of
("blue")
("gold")
("magenta")
("red")
("sky")
("violet");

void GardenBlooming::generateFlowers(){
    SpaceVect size = getMapSize();
    
    for(int y=0;y < size.y; ++y){
        for(int x=0;x<size.x; ++x){
            if(app->randomFloat(app->randomEngine) < weight){
                addFlower(SpaceVect(0.5+x,0.5+y));
            }
        }
    }
}
void GardenBlooming::addFlower(const SpaceVect& pos){
    int index = app->randomInt(app->randomEngine) % colors.size();
    const string& color = colors[index];
    
    Flower* f = new Flower(color + " flower", color, pos);
    gspace->addObject(f);
}
