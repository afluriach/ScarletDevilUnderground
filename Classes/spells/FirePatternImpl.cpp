//
//  FirePatternImpl.cpp
//  Koumachika
//
//  Created by Toni on 7/10/19.
//
//

#include "Prefix.h"

#include "Bullet.hpp"
#include "FirePatternImpl.hpp"

shared_ptr<firepattern_properties> firepattern_properties::makeSingle(
	shared_ptr<bullet_properties> props,
	SpaceFloat interval,
	SpaceFloat distance,
	SpaceFloat burstInterval,
	int burstCount
) {
	auto result = make_shared<firepattern_properties>();

	result->bullet = props;
	result->fireInterval = interval;
	result->burstInterval = burstInterval;
	result->burstCount = burstCount;

	result->emitters.push_back(bullet_emitter{
		SpaceVect::ray(distance, 0.0),
		0.0
	});

	return result;
}

shared_ptr<firepattern_properties>firepattern_properties::makeRadius(
	shared_ptr<bullet_properties> props,
	SpaceFloat interval,
	SpaceFloat distance,
	int count
) {
	auto result = make_shared<firepattern_properties>();
	SpaceFloat angleStep = float_pi * 2.0 / count;

	result->bullet = props;
	result->fireInterval = interval;
	result->rotate = false;

	result->emitters.reserve(count);

	for_irange(i, 0, count) {
		SpaceFloat a = i * angleStep;
		result->emitters.push_back(bullet_emitter{
			SpaceVect::ray(distance, a),
			a
		});
	}

	return result;
}

shared_ptr<firepattern_properties> firepattern_properties::makeSpread(
	shared_ptr<bullet_properties> props,
	SpaceFloat interval,
	SpaceFloat distance,
	SpaceFloat burstInterval,
	int burstCount,
	SpaceFloat sideAngle,
	int emitterCount
) {
	auto result = make_shared<firepattern_properties>();
	SpaceFloat spread = sideAngle * 2.0;
	SpaceFloat angleStep = spread / (emitterCount - 1);

	result->bullet = props;
	result->fireInterval = interval;
	result->burstInterval = burstInterval;
	result->burstCount = burstCount;

	for_irange(i, 0, emitterCount) {
		SpaceFloat a =  -sideAngle + i*angleStep;
		result->emitters.push_back(bullet_emitter{
			SpaceVect::ray(distance, a),
			a
		});

	}

	return result;
}

FirePatternImpl::FirePatternImpl(
	Agent *const agent,
	shared_ptr<firepattern_properties> props
) :
	FirePattern(agent),
	props(props)
{
}

void FirePatternImpl::update()
{
	timerDecrement(cooldownTimer);
}

bool FirePatternImpl::fire()
{
	bool success = false;

	if (cooldownTimer > 0.0f)
		return false;

	SpaceVect origin = agent->getPos();
	SpaceFloat angle = props->rotate ? agent->getAngle() : 0.0;

	for (auto emitter : props->emitters) {
		bool _s = emit(angle, emitter);
		success = success || _s;
	}

	if (success) {
		if (props->burstCount > 1 && props->burstInterval > 0.0 && crntBurstCount < props->burstCount) {
			cooldownTimer = props->burstInterval / props->burstCount;
			++crntBurstCount;
		}
		else {
			cooldownTimer = props->fireInterval;
			crntBurstCount = 0;
		}
	}

	return success;
}

bool FirePatternImpl::createBullet(SpaceVect offset, SpaceFloat angle)
{
	gobject_ref ref = agent->launchBullet(props->bullet, offset, angle);
	return ref.isFuture();
}

bool FirePatternImpl::emit(SpaceFloat angle, const bullet_emitter& be)
{
	SpaceVect _offset = be.offset.rotate(angle);
	SpaceFloat _angle = be.angle + angle;

	return createBullet( _offset, _angle);
}
