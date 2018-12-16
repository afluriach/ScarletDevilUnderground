//
//  Garden.cpp
//  Koumachika
//
//  Created by Toni on 12/1/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "Flower.h"
#include "Garden.hpp"
#include "GSpace.hpp"
#include "macros.h"

const vector<string> GardenBlooming::colors = {
	"blue",
	"gold",
	"magenta",
	"red",
	"sky",
	"violet"
};

void GardenBlooming::generateFlowers(){
    SpaceVect size = getMapSize();
    
    for_irange(y,0,size.y){
        for_irange(x,0,size.x){
            if(app->randomFloat(app->randomEngine) < weight){
                addFlower(SpaceVect(0.5+x,0.5+y));
            }
        }
    }
}
void GardenBlooming::addFlower(const SpaceVect& pos){
    int index = app->randomInt(app->randomEngine) % colors.size();
    const string& color = colors[index];
    
	gspace->createObject(
		GObject::make_object_factory<Flower>(
			color + " flower", color, pos
		)
	);
}
