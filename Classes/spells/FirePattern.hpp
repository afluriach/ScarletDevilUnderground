//
//  FirePattern.hpp
//  Koumachika
//
//  Created by Toni on 11/22/18.
//
//

#ifndef FirePattern_hpp
#define FirePattern_hpp

#include "Agent.hpp"

enum class PlayerFirePatternID : int;

class FirePattern
{
public:
	static const unordered_map<string, FirePatternGeneratorType> playerFirePatterns;
	static const SpaceFloat defaultLaunchDistance;

	inline FirePattern(Agent *const agent) : agent(agent) {}
	virtual inline ~FirePattern() {}

	GSpace* getSpace() const;

	inline virtual string iconPath() const { return ""; }
	inline virtual float getCost() const { return 0.0f; }

	virtual bool fireIfPossible();
	inline virtual void update() {}
protected:
	virtual bool fire() = 0;

	Agent *const agent;
};

#endif /* FirePattern_hpp */
