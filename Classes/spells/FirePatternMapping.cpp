//
//  FirePatternMapping.cpp
//  Koumachika
//
//  Created by Toni on 3/30/19.
//
//

#include "Prefix.h"

#include "EnemyFirePattern.hpp"
#include "FirePatternImpl.hpp"
#include "PlayerFirePattern.hpp"

template<class C>
FirePatternGeneratorType factoryAdapter()
{
	return [](Agent* agent) -> local_shared_ptr<FirePattern> {
		return make_local_shared<C>(agent);
	};
}

FirePatternGeneratorType implFactoryAdapter(string fpName)
{
	return [fpName](Agent* agent) -> local_shared_ptr<FirePattern> {
		auto props = app::getFirePattern(fpName);

		return make_local_shared<FirePatternImpl>(agent, props);
	};
}

#define fp_impl(x) { #x , implFactoryAdapter(#x) }

const unordered_map<string, FirePatternGeneratorType> FirePattern::playerFirePatterns = {
	fp_impl(MagicMissile),
	fp_impl(ScarletDaggerPattern),

	{ "Catadioptric", factoryAdapter<Catadioptric>() },
	{ "StarbowBreak", factoryAdapter<StarbowBreak>() },

	{ "ReimuWavePattern", factoryAdapter<ReimuWavePattern>() },
};
