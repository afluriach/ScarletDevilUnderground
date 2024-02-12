//
//  util.h
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#ifndef util_h
#define util_h

//Define assignment conversion between cocos and chipmunk vectors
Vec2 toCocos(const SpaceVect& rhs);
Vec2 toCocos(const IntVec2& rhs);
float toCocosAngle(SpaceFloat rads);
float fromCocosAngle(SpaceFloat cocosDegrees);
SpaceVect toSpaceVect(const Vec2& rhs);
SpaceVect toSpaceVect(const cocos2d::CCSize& rhs);
SpaceVect toSpaceVect(const IntVec2& ivec);
SpaceVect toSpaceVectWithCentering(const IntVec2& ivec);
Vec3 toVec3(const SpaceVect& rhs, float y = 0.0f);
b2Vec2 toBox2D(const SpaceVect& v);
IntVec2 toIntVector(const cocos2d::CCSize& rhs);
IntVec2 toIntVector(const SpaceVect& rhs);
IntVec2 toIntVector(const string& s);
CCSize toCCSize(const SpaceVect& rhs);

CCRect operator*(const CCRect& lhs, float rhs);
CCRect makeRect(Vec2 pos, CCSize bb);

SpaceFloat dirToPhysicsAngle(Direction d);
SpaceVect dirToVector(Direction d);
//Cannot be overloaded since it is exposed in the Lua API.
Direction toDirection(SpaceVect);
Direction angleToDirection(SpaceFloat a);
Direction stringToDirection(string str);
string directionToString(Direction d);
string floatToRoundedString(float val, float denom);

bool isNumeric(char c);
int getIntSuffix(const string& name);

SpaceFloat linearToAngularSpeed(SpaceFloat speed, SpaceFloat radius);

bool isInArea(const vector<SpaceRect>& areas, const SpaceVect& target, int index);
int getAreaIndex(const vector<SpaceRect>& areas, const SpaceRect& target);
int getAreaIndex(const vector<SpaceRect>& areas, const SpaceVect& p);
SpaceRect calculateCameraArea(const SpaceVect& pos);

SpaceFloat canonicalAngle(SpaceFloat a);

SpaceFloat toDegrees(SpaceFloat a);
SpaceFloat toRads(SpaceFloat deg);

SpaceFloat circleMomentOfInertia(SpaceFloat mass, SpaceFloat radius);
SpaceFloat rectangleMomentOfInertia(SpaceFloat mass, const SpaceVect& dim);
SpaceVect ricochetVelocity(SpaceVect v, SpaceVect n, SpaceFloat scale);

void timerDecrement(float& x);
void timerDecrement(float& x, float scale);
void timerDecrement(SpaceFloat& x);
void timerIncrement(float& x);
void timerIncrement(float& x, float scale);
void timerIncrement(SpaceFloat& x);
void timerIncrement(SpaceFloat& x, const SpaceFloat& scale);

string getNowTimestamp();

vector<string> splitString(const string& input,const string& sep);
bool isComment(const string& s);

string getTimeString(unsigned int seconds);
string floatToStringOptionalDecimal(float val);

template<typename T>
inline local_shared_ptr<T> makeSharedCopy(const T& val)
{
	auto result = make_local_shared<T>();
	*result = val;

	return result;
}

template<typename V>
V getOrDefaultLowerCase(const unordered_map<string, V>& _map, const string& _key, const V& _default)
{
	string s = boost::to_lower_copy(_key);
	auto it = _map.find(s);
	return it != _map.end() ? it->second : _default;
}

template<typename V>
V getOrDefaultUpperCase(const unordered_map<string, V>& _map, const string& _key, const V& _default)
{
	string s = boost::to_upper_copy(_key);
	auto it = _map.find(s);
	return it != _map.end() ? it->second : _default;
}

template<typename K, typename V>
V getOrDefault(const unordered_map<K, V>& _map, const K& _key, const V& _default)
{
	auto it = _map.find(_key);
	return it != _map.end() ? it->second : _default;
}

template<typename K, typename V>
V getOrDefault(const unordered_map<K, V>& _map, const K& _key)
{
	auto it = _map.find(_key);
	return it != _map.end() ? it->second : V();
}

template<typename K, typename V>
V getOrDefault(const map<K, V>& _map, const K& _key)
{
	auto it = _map.find(_key);
	return it != _map.end() ? it->second : V();
}

template<typename K, typename V>
V getOrDefault(const map<K, V>& _map, const K& _key, const V& _default)
{
	auto it = _map.find(_key);
	return it != _map.end() ? it->second : _default;
}

template<typename K, typename V>
void emplaceIfEmpty(unordered_map<K, V>& _map, const K& _key, const V& _default)
{
	auto it = _map.find(_key);
	if (it == _map.end())
		_map.insert_or_assign(_key, _default);
}

template<typename K, typename V>
void emplaceIfEmpty(unordered_map<K, V>& _map, const K& _key)
{
	auto it = _map.find(_key);
	if (it == _map.end())
		_map.insert_or_assign(_key, V());
}

template<typename K, typename V, typename A>
void emplaceIfEmpty(map<K, V, A>& _map, const K& _key)
{
	auto it = _map.find(_key);
	if (it == _map.end())
		_map.insert_or_assign(_key, V());
}

template<typename K, typename V, typename A>
void emplaceIfEmpty(map<K, V, A>& _map, const K& _key, V val)
{
	auto it = _map.find(_key);
	if (it == _map.end())
		_map.insert_or_assign(_key, val);
}

template<typename K, typename V>
void eraseEntry(multimap<K, V>& _map, pair<K,V> entry)
{
	auto keysRange = _map.equal_range(entry.first);

	for (auto it = keysRange.first; it != keysRange.second; ++it) {
		if (it->second == entry.second) {
			_map.erase(it);
			return;
		}
	}
}

template<typename T, size_t Size>
inline array<T,Size> getZeroArray()
{
	array<T, Size> result;

	for (int i = 0; i < Size; ++i) {
		result[i] = 0;
	}

	return result;
}

template<size_t size>
constexpr inline bitset<size> make_bitfield(size_t idx)
{
	bitset<size> result;
	result[idx] = true;
	return result;
}

template<typename... T>
string string_format(string s, T... args)
{
	boost::format fmt(s);
	string result = boost::str((fmt % ... % forward<T>(args)));

	return result;
}

namespace boost {
	template<>
	inline bool lexical_cast(const string& s)
	{
		if (boost::iequals(s, "true") || s == "1") return true;
		else if (boost::iequals(s, "false") || s == "0") return false;
		else throw boost::bad_lexical_cast();
	}
}

boost::icl::interval_map<float, int> makeIntervalMap(const vector<float_pair>& intervals);

//If index exceeds size of list, it will return the last element.
template<typename T>
T listAt(const list<T> l, int idx)
{
	auto it = l.begin();
	int count = 0;

	for (; it != l.end() && count < idx; ++it, ++count);

	if (it == l.end()) {
		return l.back();
	}
	else {
		return *it;
	}
}

class TimerSystem
{
public:
	static void printTimerStats(TimerTriplet,string);
	static string timerStatString(TimerTriplet _data, string name);
	static TimerTriplet getBufferStats(const boost::circular_buffer<chrono::duration<long, micro>>&);

	TimerSystem();

	void addEntry(TimerType, chrono::duration<long, micro>);
	TimerTriplet getStats(TimerType);

	map<TimerType, list<chrono::duration<long, micro>>> timerBuffer;
};

template<typename T>
struct list_set
{
	list<T, local_allocator<T>> l;

	inline void insert(const T& t)
	{
		if (!contains(t)) {
			l.push_back(t);
		}
	}

	inline void erase(const T& t)
	{
		for (auto it = l.begin(); it != l.end(); ++it) {
			if (*it == t) {
				l.erase(it);
				return;
			}
		}
	}

	inline bool contains(const T& t) const
	{
		for (auto entry : l) {
			if (entry == t)
				return true;
		}
		return false;
	}

	inline void clear()
	{
		l.clear();
	}

	inline size_t size() const
	{
		return l.size();
	}

	inline void for_each(function<void(const T&)> f) const
	{
		for (auto const& t : l) {
			f(t);
		}
	}
};

#endif /* util_h */
