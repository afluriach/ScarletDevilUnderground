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

const unordered_map<string, FirePatternGeneratorType> FirePattern::playerFirePatterns = {
	{ "MagicMissile", factoryAdapter<MagicMissile>() },
	{ "Catadioptric", factoryAdapter<Catadioptric>() },
	{ "ScarletDagger", factoryAdapter<ScarletDaggerPattern>() },
	{ "StarbowBreak", factoryAdapter<StarbowBreak>() },
};
