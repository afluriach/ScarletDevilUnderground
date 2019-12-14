//
//  FirePatternMapping.cpp
//  Koumachika
//
//  Created by Toni on 3/30/19.
//
//

#include "Prefix.h"

#include "FirePatternImpl.hpp"
#include "GSpace.hpp"
#include "PlayerFirePattern.hpp"
#include "xml.hpp"

template<class C>
FirePatternGeneratorType factoryAdapter()
{
	return [](Agent* agent) -> shared_ptr<FirePattern> {
		return make_shared<C>(agent);
	};
}

FirePatternGeneratorType implFactoryAdapter(string fpName)
{
	return [fpName](Agent* agent) -> shared_ptr<FirePattern> {
		auto props = app::getFirePattern(fpName);

		return make_shared<FirePatternImpl>(agent, props);
	};
}

#define fp_impl(x) { #x , implFactoryAdapter(#x) }

const unordered_map<string, FirePatternGeneratorType> FirePattern::playerFirePatterns = {
	fp_impl(MagicMissile),
	fp_impl(ScarletDaggerPattern),

	{ "Catadioptric", factoryAdapter<Catadioptric>() },
	{ "StarbowBreak", factoryAdapter<StarbowBreak>() },
};
