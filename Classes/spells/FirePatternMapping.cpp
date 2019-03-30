//
//  FirePatternMapping.cpp
//  Koumachika
//
//  Created by Toni on 3/30/19.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "PlayerFirePattern.hpp"

template<class C>
FirePatternGeneratorType factoryAdapter()
{
	return [](Agent* agent, int level) -> shared_ptr<FirePattern> {
		return make_shared<C>(agent, level);
	};
}

const unordered_map<PlayerFirePatternID, FirePatternGeneratorType> FirePattern::playerFirePatterns = {
	{ PlayerFirePatternID::catadioptric, factoryAdapter<Catadioptric>() },
	{ PlayerFirePatternID::scarletDagger, factoryAdapter<ScarletDaggerPattern>() },
	{ PlayerFirePatternID::starbowBreak, factoryAdapter<StarbowBreak>() },
};
