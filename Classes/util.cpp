//
//  util.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "enum.h"
#include "macros.h"
#include "util.h"

const SpaceFloat primaryAngles[4] = {0.0, float_pi * 0.5, float_pi, float_pi * 1.5};

Scene* crntScene()
{
    return Director::getInstance()->getRunningScene();
}


SpaceFloat dirToPhysicsAngle(Direction d)
{
    if(d == Direction::none) return 0.0;
    
    return primaryAngles[to_int(d)-1];
}

SpaceVect dirToVector(Direction d)
{
    return SpaceVect::ray(1.0, dirToPhysicsAngle(d));
}

//cocos Vector uses atan2, which returns angle in range [-pi,pi]
Direction toDirection(SpaceVect v)
{
    if(v.x == 0.0 && v.y == 0.0)
        return Direction::none;
    
    return angleToDirection(toCocos(v).getAngle());
}

//round to nearest primary direction
Direction angleToDirection(SpaceFloat a)
{
    int closest = 0;
    SpaceFloat distance = float_pi;
    SpaceFloat angle = canonicalAngle(a);
    
    for_irange(i,0,4){
        SpaceFloat crnt = abs(angle - primaryAngles[i]);
        
        if(crnt < distance){
            closest = i;
            distance = crnt;
        }
    }
    
    return static_cast<Direction>(closest+1);
}

SpaceFloat circleMomentOfInertia(SpaceFloat mass, SpaceFloat radius)
{
    return float_pi/2*pow(radius,4);
}

SpaceFloat rectangleMomentOfInertia(SpaceFloat mass, const SpaceVect& dim)
{
    return mass*(dim.x*dim.x+dim.y*dim.y)/12;
}

#define enum_strcmp(val) if(str == #val) return Direction::val;
Direction stringToDirection(string str)
{
    enum_strcmp(up)
    enum_strcmp(right)
    enum_strcmp(left)
    enum_strcmp(down)
    
    log("Invalid direction: %s", str.c_str());
    return Direction::none;
}

const array<string, to_size_t(Direction::end)> directionNames = {
	"none",
	"right",
	"up",
	"left",
	"down"
};

string directionToString(Direction d)
{
	return (d >= Direction::none && d < Direction::end) ? directionNames[to_size_t(d)] : "";
}

IntVec2 toIntVector(const cocos2d::CCSize& rhs)
{
    return IntVec2(rhs.width,rhs.height);
}

IntVec2 toIntVector(const SpaceVect& rhs)
{
	return IntVec2(rhs.x, rhs.y);
}


Vec2 toCocos(const SpaceVect& rhs)
{
    return Vec2(expand_vector2(rhs));
}

Vec2 toCocos(const IntVec2& rhs)
{
	return Vec2(rhs.first, rhs.second);
}


SpaceVect toChipmunk(const Vec2& rhs)
{
    return SpaceVect(expand_vector2(rhs));
}

SpaceVect toChipmunk(const cocos2d::CCSize& rhs)
{
    return SpaceVect(rhs.width, rhs.height);
}

FMOD_VECTOR toFmod(const SpaceVect& rhs)
{
	return { to_float(rhs.x), 0.0f, to_float(rhs.y) };
}

string getRealPath(const string& path)
{
    return FileUtils::getInstance()->fullPathForFilename(path);
}

string loadTextFile(const string& res)
{
    return FileUtils::getInstance()->getStringFromFile(res);
}

vector<string> splitString(const string& input,const string& sep)
{
    vector<string> output;
    boost::split(output, input,boost::is_any_of(sep));
    return output;
}

SpaceFloat canonicalAngle(SpaceFloat a)
{
    return a - float_2pi * floor( a / float_2pi);
}

SpaceFloat toDegrees(SpaceFloat a)
{
	return a / float_pi * 180.0;
}

void timerDecrement(boost::rational<int>& x)
{
	x = max(x - App::secondsPerFrameRational, boost::rational<int>(0));
}

void timerDecrement(float& x)
{
	x = max(x - to_float(App::secondsPerFrame), 0.0f);
}

void timerDecrement(SpaceFloat& x)
{
	x = max(x - App::secondsPerFrame, 0.0);
}

void timerIncrement(SpaceFloat& x)
{
	x += App::secondsPerFrame;
}

TimerSystem::TimerSystem()
{
	enum_foreach(TimerType, _type, begin, end)
	{
		timerBuffer.insert_or_assign(_type, list<chrono::duration<long, micro>>());
	}
}

void TimerSystem::printTimerStats(tuple<long, long, long> _data, string name)
{
	log("Timer type %s: min: %ld, avg: %ld, max: %ld.", name.c_str(), get<0>(_data), get<1>(_data), get<2>(_data));
}

void TimerSystem::addEntry(TimerType _type, chrono::duration<long, micro> _us)
{
	list<chrono::duration<long, micro>>& _l = timerBuffer.at(_type);

	while (_l.size() >= App::framesPerSecond) {
		_l.pop_front();
	}
	_l.push_back(_us);
}

tuple<long, long, long> TimerSystem::getBufferStats(const boost::circular_buffer<chrono::duration<long, micro>>& buffer)
{
	chrono::microseconds accumulator(0);
	chrono::microseconds _min(1000000);
	chrono::microseconds _max(0);

	for (chrono::microseconds entry : buffer) {
		accumulator += entry;

		if (entry < _min)
			_min = entry;
		if (entry > _max)
			_max = entry;
	}

	if (accumulator != chrono::microseconds(0) && buffer.size() > 0) {
		accumulator /= buffer.size();
	}

	return tuple<long, long, long>(
		chrono::duration_cast<chrono::microseconds>(_min).count(),
		chrono::duration_cast<chrono::microseconds>(accumulator).count(),
		chrono::duration_cast<chrono::microseconds>(_max).count()
	);
}

tuple<long, long, long> TimerSystem::getStats(TimerType _type)
{
	chrono::microseconds accumulator(0);
	chrono::microseconds _min(1000000);
	chrono::microseconds _max(0);

	for (chrono::microseconds entry : timerBuffer.at(_type)) {
		accumulator += entry;

		if (entry < _min)
			_min = entry;
		if (entry > _max)
			_max = entry;
	}

	if (accumulator != chrono::microseconds(0) && timerBuffer.at(_type).size() > 0) {
		accumulator /= timerBuffer.at(_type).size();
	}

	return tuple<long, long, long>(
		chrono::duration_cast<chrono::microseconds>(_min).count(),
		chrono::duration_cast<chrono::microseconds>(accumulator).count(),
		chrono::duration_cast<chrono::microseconds>(_max).count()
	);
}
