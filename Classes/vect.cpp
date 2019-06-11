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

cpFloat operator*(const SpaceVect& lhs, const SpaceVect& rhs) {
	return lhs.x * rhs.x +lhs.y * rhs.y;
}

SpaceVect operator*(const SpaceVect& lhs, const cpFloat v) {
	return SpaceVect(lhs.x * v, lhs.y * v);
}

SpaceVect operator/(const SpaceVect& lhs, const cpFloat v) {
	return SpaceVect(lhs.x / v, lhs.y / v);
}

SpaceVect operator*(const cpFloat v, const SpaceVect& rhs) {
	return SpaceVect(rhs.x * v, rhs.y * v);
}

SpaceVect operator/(const cpFloat v, const SpaceVect& rhs) {
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

SpaceVect::SpaceVect(cpFloat x, cpFloat y) : x(x), y(y) {}

SpaceVect::SpaceVect(const cpVect& vect) : x(vect.x), y(vect.y) {}

SpaceVect::operator cpVect() const {
	return cpv(x, y);
}

SpaceVect& SpaceVect::operator/=(const cpFloat v) {
	x /= v;
	y /= v;
	return *this;
}

SpaceVect& SpaceVect::operator*=(const cpFloat v) {
	x *= v;
	y *= v;
	return *this;
}

SpaceVect& SpaceVect::operator-=(const SpaceVect& rhs) {
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

SpaceVect& SpaceVect::operator-=(const cpFloat v) {
	x -= v;
	y -= v;
	return *this;
}

SpaceVect& SpaceVect::operator+=(const cpFloat v) {
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

cpFloat SpaceVect::lengthSq() const {
	return x*x+y*y;
}

cpFloat SpaceVect::length() const {
	return sqrt(lengthSq());
}
    
SpaceVect SpaceVect::limit(cpFloat _limit) const
{
	if (lengthSq() > _limit*_limit) {
		return normalizeSafe() * _limit;
	}
	else {
		return *this;
	}
}

SpaceVect SpaceVect::setMag(cpFloat mag) const
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

SpaceVect SpaceVect::clamp(cpFloat len) const {
	return (lengthSq() > len*len) ? normalize() * len : *this;
}

cpFloat SpaceVect::toAngle() const {
	return cpfatan2(y, x);
}

cpFloat SpaceVect::dot(SpaceVect v1, SpaceVect v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

cpFloat SpaceVect::cross(SpaceVect v1, SpaceVect v2) {
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

cpFloat SpaceVect::getMax() const {
	return max(x, y);
}

SpaceVect SpaceVect::rotate(cpFloat angleRadians) const {
    double _cos = cos(angleRadians);
    double _sin = sin(angleRadians);
    return SpaceVect(
        _cos * x - _sin * y,
        _sin * x + _cos * y
    );
}

SpaceVect SpaceVect::lerp(SpaceVect v1, SpaceVect v2, cpFloat t) {
	return v1 * (1 - t) + v2 * t;
}

SpaceVect SpaceVect::lerpconst(SpaceVect v1, SpaceVect v2, cpFloat d) {
	return v1 + (v2 - v1).clamp( d);
}

SpaceVect SpaceVect::slerp(SpaceVect v1, SpaceVect v2, cpFloat t) {
	auto tmp = cpvslerp(v1, v2, t);
	return SpaceVect(tmp.x, tmp.y);
}

SpaceVect SpaceVect::slerpconst(SpaceVect v1, SpaceVect v2, cpFloat a) {
	auto tmp = cpvslerpconst(v1, v2, a);
	return SpaceVect(tmp.x, tmp.y);
}

cpFloat SpaceVect::dist(SpaceVect v1, SpaceVect v2) {
	return (v1-v2).length();
}

cpFloat SpaceVect::distSq(SpaceVect v1, SpaceVect v2) {
	return (v1-v2).lengthSq();
}

bool SpaceVect::fuzzyMatch(SpaceVect v1, SpaceVect v2) {
	return distSq(v1, v2) < 1e-4;
}

SpaceVect SpaceVect::forAngle(cpFloat a) {
	return SpaceVect(cpfcos(a), cpfsin(a));
}
    
const SpaceVect SpaceVect::zero = SpaceVect(0.0f,0.0f);
const SpaceVect SpaceVect::unit_square = SpaceVect(1.0f,1.0f);


std::ostream& operator<<(std::ostream& out, const SpaceVect& vec)
{
	int w = static_cast<int>(out.width(0));
	out << "(" << std::setw(w) << vec.x << ", " << std::setw(w) << vec.y << ")";
	return out;
}
