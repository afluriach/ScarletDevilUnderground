//
//  Fairy.hpp
//  Koumachika
//
//  Created by Toni on 11/30/18.
//
//

#ifndef Fairy_hpp
#define Fairy_hpp

#include "Collectibles.hpp"
#include "Enemy.hpp"

class Fairy2 : public Enemy
{
public:

	enum class ai_state {
		normal,
		flee,
		fleeWithSupport,
		supportOffered,
		supporting
	};

	enum class ai_priority {
		engage,
		support,
		flee,
	};

	static const string baseAttributes;
	static const float lowHealthRatio;

	Fairy2(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual inline SpaceFloat getRadarRadius() const { return 6.0; }
	virtual inline SpaceFloat getDefaultFovAngle() const { return float_pi / 4.0; }

	inline string getSprite() const { return "redFairy2"; }
	virtual inline string getTypeName() const { return "Fairy2"; }

	virtual inline string initStateMachine() { return "fairy2"; }
	void addFleeThread();
	void addSupportThread(gobject_ref other);
	void removeSupportThread();

	virtual void update();

	void requestSupport();
	gobject_ref requestHandler(gobject_ref other);
	void responseHandler(gobject_ref supporting);
	void acknowledgeHandaler(gobject_ref supportTarget);
protected:
	ai_state crntState = ai_state::normal;
	shared_ptr<ai::Thread> supportThread = 0;
	gobject_ref supportingAgent;
};

#endif /* Fairy_hpp */
