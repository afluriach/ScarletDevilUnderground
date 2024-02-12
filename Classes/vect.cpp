//
//  vect.cpp
//  Koumachika
//
//  Created by Toni on 2/9/19.
//
//

#include "Prefix.h"

#include "vect.hpp"

bool operator==(const SpaceVect& lhs, const SpaceVect& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(const SpaceVect& lhs, const SpaceVect& rhs) {
	return lhs.x != rhs.x || lhs.y != rhs.y;
}

SpaceFloat operator*(const SpaceVect& lhs, const SpaceVect& rhs) {
	return lhs.x * rhs.x +lhs.y * rhs.y;
}

SpaceVect operator*(const SpaceVect& lhs, const SpaceFloat v) {
	return SpaceVect(lhs.x * v, lhs.y * v);
}

SpaceVect operator/(const SpaceVect& lhs, const SpaceFloat v) {
	return SpaceVect(lhs.x / v, lhs.y / v);
}

SpaceVect operator*(const SpaceFloat v, const SpaceVect& rhs) {
	return SpaceVect(rhs.x * v, rhs.y * v);
}

SpaceVect operator/(const SpaceFloat v, const SpaceVect& rhs) {
	return SpaceVect(rhs.x / v, rhs.y / v);
}

SpaceVect operator-(const SpaceVect& lhs, const SpaceVect& rhs) {
	return SpaceVect(lhs.x - rhs.x, lhs.y - rhs.y);
}

SpaceVect operator+(const SpaceVect& lhs, const SpaceVect& rhs) {
	return SpaceVect(lhs.x + rhs.x, lhs.y + rhs.y);
}

bool operator<(const SpaceVect& lhs, const SpaceVect& rhs) {
	return lhs.x < rhs.x && lhs.y < rhs.y;
}

bool operator>(const SpaceVect& lhs, const SpaceVect& rhs) {
	return lhs.x > rhs.x && lhs.y > rhs.y;
}

SpaceVect::SpaceVect() : x(0), y(0) {}

SpaceVect::SpaceVect(SpaceFloat x, SpaceFloat y) : x(x), y(y) {}

SpaceVect::SpaceVect(const SpaceVect& vect) : x(vect.x), y(vect.y) {}

SpaceVect& SpaceVect::operator/=(const SpaceFloat v) {
	x /= v;
	y /= v;
	return *this;
}

SpaceVect& SpaceVect::operator*=(const SpaceFloat v) {
	x *= v;
	y *= v;
	return *this;
}

SpaceVect& SpaceVect::operator-=(const SpaceVect& rhs) {
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

SpaceVect& SpaceVect::operator-=(const SpaceFloat v) {
	x -= v;
	y -= v;
	return *this;
}

SpaceVect& SpaceVect::operator+=(const SpaceFloat v) {
	x += v;
	y += v;
	return *this;
}

SpaceVect& SpaceVect::operator+=(const SpaceVect& rhs) {
	x += rhs.x;
	y += rhs.y;
	return *this;
}

SpaceVect& SpaceVect::operator++() {
	++x;
	++y;
	return *this;
}

SpaceVect& SpaceVect::operator--() {
	--x;
	--y;
	return *this;
}

SpaceFloat SpaceVect::lengthSq() const {
	return x*x+y*y;
}

SpaceFloat SpaceVect::length() const {
	return sqrt(lengthSq());
}
    
SpaceVect SpaceVect::limit(SpaceFloat _limit) const
{
	if (lengthSq() > _limit*_limit) {
		return normalizeSafe() * _limit;
	}
	else {
		return *this;
	}
}

SpaceVect SpaceVect::setMag(SpaceFloat mag) const
{
	if (isZero())
		return *this;
	else {
		return normalizeSafe() * mag;
	}
}

bool SpaceVect::isZero() const {
    return x == 0 && y == 0;
}

SpaceVect SpaceVect::perp() const {
	return SpaceVect(-y, x);
}

SpaceVect SpaceVect::rperp() const {
	return SpaceVect(y, -x);
}

SpaceVect SpaceVect::normalize() const {
	return *this/length();
}

SpaceVect SpaceVect::normalizeSafe() const {
	if (x == 0 && y == 0) {
		return SpaceVect();
	} else {
		return normalize();
	}
}

SpaceVect SpaceVect::clamp(SpaceFloat len) const {
	return (lengthSq() > len*len) ? normalize() * len : *this;
}

SpaceFloat SpaceVect::toAngle() const {
    if(lengthSq() >= atan_limit_sq)
        return atan2(y, x);
    else
        return 0.0;
}

SpaceFloat SpaceVect::dot(SpaceVect v1, SpaceVect v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

SpaceFloat SpaceVect::cross(SpaceVect v1, SpaceVect v2) {
	return v1.x*v2.y - v1.y*v2.x;
}

SpaceVect SpaceVect::project(SpaceVect v1, SpaceVect v2) {
	return SpaceVect::dot(v1, v2) / v2.lengthSq() * v2;
}

SpaceVect SpaceVect::rotate(SpaceVect v) const {
	return SpaceVect(x * v.x - y * v.y, x * v.y + y * v.x);
}

SpaceVect SpaceVect::unrotate(SpaceVect v) const {
	return SpaceVect(x * v.x + y * v.y, y * v.x - x * v.y);
}

SpaceVect SpaceVect::roundToNearestDirection(int numSlices) const{
	double _len = length();
	double _step = 1.0 / numSlices;

	int closest = -1;
	double distance = _len * 2.0;

	for_irange(i, 0, numSlices)
	{
		SpaceVect v = ray(_len, float_pi*2.0 * i * _step);

		double dist = (*this - v).length();
		if (dist < distance) {
			distance = dist;
			closest = i;
		}
	}

	if (closest == -1) return SpaceVect::zero;
	else return ray(_len, float_pi*2.0 * closest * _step);
}

SpaceFloat SpaceVect::getMax() const {
	return max(x, y);
}

SpaceVect SpaceVect::rotate(SpaceFloat angleRadians) const {
    double _cos = cos(angleRadians);
    double _sin = sin(angleRadians);
    return SpaceVect(
        _cos * x - _sin * y,
        _sin * x + _cos * y
    );
}

SpaceVect SpaceVect::lerp(SpaceVect v1, SpaceVect v2, SpaceFloat t) {
	return v1 * (1 - t) + v2 * t;
}

SpaceVect SpaceVect::lerpconst(SpaceVect v1, SpaceVect v2, SpaceFloat d) {
	return v1 + (v2 - v1).clamp( d);
}

SpaceFloat SpaceVect::dist(SpaceVect v1, SpaceVect v2) {
	return (v1-v2).length();
}

SpaceFloat SpaceVect::distSq(SpaceVect v1, SpaceVect v2) {
	return (v1-v2).lengthSq();
}

bool SpaceVect::fuzzyMatch(SpaceVect v1, SpaceVect v2) {
	return distSq(v1, v2) < 1e-4;
}

SpaceVect SpaceVect::forAngle(SpaceFloat a) {
	return SpaceVect(cos(a), sin(a));
}
    
const SpaceVect SpaceVect::zero = SpaceVect(0.0f,0.0f);
const SpaceVect SpaceVect::unit_square = SpaceVect(1.0f,1.0f);


std::ostream& operator<<(std::ostream& out, const SpaceVect& vec)
{
	int w = static_cast<int>(out.width(0));
	out << "(" << std::setw(w) << vec.x << ", " << std::setw(w) << vec.y << ")";
	return out;
}
