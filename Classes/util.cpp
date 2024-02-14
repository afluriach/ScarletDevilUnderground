//
//  util.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

SpaceFloat dirToPhysicsAngle(Direction d)
{
    if(d == Direction::none) return 0.0;
    
    return (to_int(d)-1)*float_pi*0.5;
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

bool isValidDirection(Direction d)
{
    return
        d == Direction::up ||
        d == Direction::down ||
        d == Direction::left ||
        d == Direction::right
    ;
}

Direction invertDirection(Direction d)
{
    Direction result = Direction::none;
    
    switch(d)
    {
        case Direction::up:
            result = Direction::down;
        break;
        case Direction::down:
            result = Direction::up;
        break;
        case Direction::left:
            result = Direction::right;
        break;
        case Direction::right:
            result = Direction::left;
        break;
    }
    
    if(result == Direction::none){
        log1("Invalid direction %d provided!", to_int(d));
    }
    
    return result;
}

//round to nearest primary direction
Direction angleToDirection(SpaceFloat a)
{
    SpaceFloat angle = canonicalAngle(a+float_pi*0.25);

    int i = angle * pi_inv * 2.0;
    i = i > 3 ? 3 : i;

    return static_cast<Direction>(i+1);
}

SpaceFloat circleMomentOfInertia(SpaceFloat mass, SpaceFloat radius)
{
    return float_pi/2*pow(radius,4);
}

SpaceFloat rectangleMomentOfInertia(SpaceFloat mass, const SpaceVect& dim)
{
    return mass*(dim.x*dim.x+dim.y*dim.y)/12;
}

SpaceVect ricochetVelocity(SpaceVect v, SpaceVect n, SpaceFloat scale)
{
	return (-2.0 * (SpaceVect::dot(v, n))*n + v)*scale;
}

#define enum_strcmp(val) if(str == #val) return Direction::val;
Direction stringToDirection(string str)
{
    enum_strcmp(up)
    enum_strcmp(right)
    enum_strcmp(left)
    enum_strcmp(down)
	enum_strcmp(none)
    
    log1("Invalid direction: %s", str.c_str());
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

string floatToRoundedString(float val, float denom)
{
	float roundedVal = floor(val * denom) / denom;
	return boost::lexical_cast<string>(roundedVal);
}

vector<SpaceVect> getPoints(SpaceVect start, SpaceVect dir, int count)
{
    vector<SpaceVect> result;

    for(int i = 0; i < count; ++i){
        result.push_back(start + dir*i);
    }
    
    return result;
}

vector<SpaceVect> getRow(SpaceFloat row, SpaceFloat start, SpaceFloat last)
{
    vector<SpaceVect> result;

    for(SpaceFloat col = start; col <= last; col += 1.0){
        result.push_back(SpaceVect(col, row));
    }
    
    return result;
}

vector<SpaceVect> getColumn(SpaceFloat column, SpaceFloat start, SpaceFloat last)
{
    vector<SpaceVect> result;

    for(SpaceFloat row = start; row <= last; row += 1.0){
        result.push_back(SpaceVect(column, row));
    }
    
    return result;
}

vector<SpaceVect> getAdjacentTiles(GObject* object, Direction direction)
{
    if(!isValidDirection(direction)){
        log1("Invalid Direction %d!", to_int(direction));
        return vector<SpaceVect>();
    }

    SpaceVect center = object->getPos();
    SpaceVect dim = object->getDimensions();
    
    //the tiles that the object occupies
    SpaceFloat startCol = center.x - dim.x/2 + 0.5;
    SpaceFloat endCol = center.x + dim.x/2 - 0.5;
    SpaceFloat startRow = center.y - dim.y/2 + 0.5;
    SpaceFloat endRow = center.y + dim.y/2 - 0.5;
    
    switch(direction)
    {
    case Direction::up:
        return getRow(endRow + 1.0, startCol, endCol);
    case Direction::down:
        return getRow(startRow - 1.0, startCol, endCol);
    case Direction::left:
        return getColumn(startCol - 1.0, startRow, endRow);
    case Direction::right:
        return getColumn(endCol + 1.0, startRow, endRow);
    }
}

bool isNumeric(char c)
{
    return c >= '0' && c <= '9';
}

int getIntSuffix(const string& name)
{
    if(name.empty()){
        log("Empty string.");
        return -1;
    }
    
    int lastIdx = name.size() - 1;
    int idx = lastIdx;
    while(isNumeric(name[idx]))
        --idx;
        
    if(idx == lastIdx ){
        log("Name does not have a numeric suffix");
        return -1;
    }
    
    try{
        return boost::lexical_cast<int>(name.substr(idx, lastIdx));
    }
    catch(boost::bad_lexical_cast){
        log("Error parsing numeric suffix");
    }
    
    return -1;
}

SpaceFloat linearToAngularSpeed(SpaceFloat speed, SpaceFloat radius)
{
	SpaceFloat circumference = 2.0 * float_pi * radius;
	return circumference > 0.0 ? speed / circumference : 0.0;
}

IntVec2 toIntVector(const cocos2d::CCSize& rhs)
{
    return IntVec2(rhs.width,rhs.height);
}

IntVec2 toIntVector(const SpaceVect& rhs)
{
	return IntVec2(rhs.x, rhs.y);
}

IntVec2 toIntVector(const string& s)
{
	vector<string> tokens = splitString(s, ",");

	return IntVec2(
		boost::lexical_cast<int>(tokens[0]),
		boost::lexical_cast<int>(tokens[1])
	);
}

CCSize toCCSize(const SpaceVect& rhs)
{
	return CCSize(rhs.x, rhs.y);
}

Vec2 toCocos(const SpaceVect& rhs)
{
    return Vec2(expand_vector2(rhs));
}

Vec2 toCocos(const IntVec2& rhs)
{
	return Vec2(rhs.first, rhs.second);
}

float toCocosAngle(SpaceFloat rads)
{
	return 90.0f - toDegrees(rads);
}

float fromCocosAngle(SpaceFloat cocosDegrees)
{
	return toRads(90.0f - cocosDegrees);
}

SpaceVect toSpacVect(const Vec2& rhs)
{
    return SpaceVect(expand_vector2(rhs));
}

SpaceVect toSpaceVect(const cocos2d::CCSize& rhs)
{
    return SpaceVect(rhs.width, rhs.height);
}

SpaceVect toSpaceVect(const IntVec2& ivec)
{
	return SpaceVect(ivec.first, ivec.second);
}

SpaceVect toSpaceVectWithCentering(const IntVec2& ivec)
{
	return SpaceVect(ivec.first + 0.5, ivec.second + 0.5);
}

Vec3 toVec3(const SpaceVect& rhs, float y)
{
	return { to_float(rhs.x), y, to_float(rhs.y) };
}

b2Vec2 toBox2D(const SpaceVect& v)
{
	return b2Vec2(v.x, v.y);
}

CCRect operator*(const CCRect& lhs, float rhs)
{
	return CCRect(lhs.getMinX()*rhs, lhs.getMinY()*rhs, lhs.getWidth()*rhs, lhs.getHeight()*rhs);
}

CCRect makeRect(Vec2 pos, CCSize bb)
{
	return CCRect(
		pos.x - bb.width*0.5f,
		pos.y - bb.height*0.5f,
		bb.width,
		bb.height
	);
}

vector<string> splitString(const string& input,const string& sep)
{
    vector<string> output;
    boost::split(output, input,boost::is_any_of(sep));
	output.erase(
		remove_if(
			output.begin(),
			output.end(),
			[](string s) -> bool { return s.empty(); }
		),
		output.end()
	);
    return output;
}

bool isComment(const string& s)
{
	return boost::starts_with(s, "#") || boost::starts_with(s, "//");
}

string getTimeString(unsigned int seconds)
{
	unsigned int minutes = seconds / 60;
	unsigned int hours = minutes / 60;

	seconds %= 60;
	minutes %= 60;

	return boost::str(
		boost::format("%02u:%02u:%02u	") % 
		hours %
		minutes %
		seconds
	);
}

string floatToStringOptionalDecimal(float val)
{
	if (val == floor(val)) {
		return boost::str(boost::format("%.0f") % val);
	}
	else {
		return boost::str(boost::format("%.2f") % val);
	}
}

bool isInArea(const vector<SpaceRect>& areas, const SpaceVect& target, int index)
{
	if (index == -1) {
		return true;
	}

	SpaceRect mapArea = areas.at(index);
	return mapArea.containsPoint(target);
}

int getAreaIndex(const vector<SpaceRect>& areas, const SpaceRect& target)
{
	SpaceVect targetLL = target.getLLCorner();
	SpaceVect targetUR = target.getURCorner();

	for_irange(i, 0, areas.size())
	{
		SpaceRect mapArea = areas.at(i);

		SpaceVect crntLL = mapArea.getLLCorner();
		SpaceVect crntUR = mapArea.getURCorner();

		if (targetLL.x >= crntLL.x && targetUR.x <= crntUR.x &&
			targetLL.y >= crntLL.y && targetUR.y <= crntUR.y) {
			return i;
		}
	}
	return -1;
}

int getAreaIndex(const vector<SpaceRect>& areas, const SpaceVect& target)
{
	for_irange(i, 0, areas.size())
	{
		SpaceRect mapArea = areas.at(i);

		if (mapArea.containsPoint(target)) {
			return i;
		}
	}
	return -1;
}

SpaceRect calculateCameraArea(const SpaceVect& pos)
{
	SpaceFloat heightRatio = 1.0f * app::params.height / app::params.width;
	return SpaceRect(pos, SpaceVect(app::viewWidth, app::viewWidth*heightRatio));
}

SpaceFloat canonicalAngle(SpaceFloat a)
{
	SpaceFloat result = a;

	while (result < 0)
		result += float_pi * 2.0;
	while (result >= float_pi * 2.0)
		result -= float_pi * 2.0;

	return result;

//   return a - float_2pi * floor( a / float_2pi);
//	return fmod(a, float_pi*2.0);
}

SpaceFloat toDegrees(SpaceFloat a)
{
	return a / float_pi * 180.0;
}

SpaceFloat toRads(SpaceFloat deg)
{
	return deg * float_pi / 180.0;
}

void timerDecrement(float& x)
{
	x = max(x - to_float(app::params.secondsPerFrame), 0.0f);
}

void timerDecrement(float& x, float scale)
{
	x = max(x - to_float(app::params.secondsPerFrame*scale), 0.0f);
}

void timerDecrement(SpaceFloat& x)
{
	x = max(x - app::params.secondsPerFrame, 0.0);
}

void timerIncrement(float& x)
{
	x += app::params.secondsPerFrame;
}

void timerIncrement(float& x, float scale)
{
	x += app::params.secondsPerFrame * scale;
}

void timerIncrement(SpaceFloat& x)
{
	x += app::params.secondsPerFrame;
}

void timerIncrement(SpaceFloat& x, const SpaceFloat& scale)
{
	x += app::params.secondsPerFrame*scale;
}

string getNowTimestamp()
{
	char buf[32];
	chrono::system_clock::time_point now = chrono::system_clock::now();
	time_t timepoint = chrono::system_clock::to_time_t(now);
	//return asctime(localtime(&timepoint));
	strftime(buf, 64, "%Y-%m-%d %H-%M-%S", localtime(&timepoint));
	return string(buf);
}

boost::icl::interval_map<float, int> makeIntervalMap(const vector<float_pair>& intervals)
{
	boost::icl::interval_map<float, int> result;
	int intervalCount = 1;

	for (float_pair entry : intervals) {
		result += make_pair(
			boost::icl::interval<float>::right_open(entry.first, entry.second),
			intervalCount++
		);
	}

	return result;
}

TimerSystem::TimerSystem()
{
	enum_foreach(TimerType, _type, begin, end)
	{
		timerBuffer.insert_or_assign(_type, list<chrono::duration<long, micro>>());
	}
}

void TimerSystem::printTimerStats(TimerTriplet _data, string name)
{
	log4("Timer type %s: min: %ld, avg: %ld, max: %ld.", name.c_str(), get<0>(_data), get<1>(_data), get<2>(_data));
}

string TimerSystem::timerStatString(TimerTriplet _data, string name)
{
	stringstream ss;
	ss.precision(1);

	ss << fixed << name << ": " << get<0>(_data)/1000.0f <<  " / " << get<1>(_data) / 1000.0f << " / " << get<2>(_data) / 1000.0f;

	return ss.str();
}

void TimerSystem::addEntry(TimerType _type, chrono::duration<long, micro> _us)
{
	list<chrono::duration<long, micro>>& _l = timerBuffer.at(_type);

	while (_l.size() >= app::params.framesPerSecond) {
		_l.pop_front();
	}
	_l.push_back(_us);
}

TimerTriplet TimerSystem::getBufferStats(const boost::circular_buffer<chrono::duration<long, micro>>& buffer)
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

	return TimerTriplet(
		chrono::duration_cast<chrono::microseconds>(_min).count(),
		chrono::duration_cast<chrono::microseconds>(accumulator).count(),
		chrono::duration_cast<chrono::microseconds>(_max).count()
	);
}

TimerTriplet TimerSystem::getStats(TimerType _type)
{
	if (timerBuffer.at(_type).empty()) {
		return TimerTriplet(0, 0, 0);
	}

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

	return TimerTriplet(
		chrono::duration_cast<chrono::microseconds>(_min).count(),
		chrono::duration_cast<chrono::microseconds>(accumulator).count() / timerBuffer.at(_type).size(),
		chrono::duration_cast<chrono::microseconds>(_max).count()
	);
}

