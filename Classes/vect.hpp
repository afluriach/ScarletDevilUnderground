//
//  vect.hpp
//  Koumachika
//
//  Created by Toni on 2/9/19.
//
//

#ifndef vect_hpp
#define vect_hpp

class SpaceVect {
public:
	/// Constructor
	SpaceVect(cpFloat x, cpFloat y);

	/// Creates the zero vector.
	SpaceVect();

	/// Copy constructor
	SpaceVect(const cpVect&);

	/// Conversion operator to Chipmunk's cpVect.
	operator cpVect() const;

	SpaceVect& operator/=(const cpFloat v);
	SpaceVect& operator*=(const cpFloat v);
	SpaceVect& operator-=(const SpaceVect& rhs);
	SpaceVect& operator-=(const cpFloat v);
	SpaceVect& operator+=(const cpFloat v);
	SpaceVect& operator+=(const SpaceVect& rhs);
	SpaceVect& operator++();
	SpaceVect& operator--();

	/// Returns the length.
	cpFloat length() const;

	/// Returns the squared length. Faster than SpaceVect::length() when you only need to compare lengths.
	cpFloat lengthSq() const;
        
	SpaceVect limit(cpFloat _limit) const;
	SpaceVect setMag(cpFloat mag) const;

    bool isZero() const;

	/// Returns a perpendicular vector. (90 degree rotation)
	SpaceVect perp() const;

	/// Returns a perpendicular vector. (-90 degree rotation)
	SpaceVect rperp() const;

	/// Returns a normalized copy.
	SpaceVect normalize() const;

	/// Returns a normalized copy or the zero vector if the vector was already the zero vector. Protects against
	/// divide by zero errors.
	SpaceVect normalizeSafe() const;

	/// Clamp v to length len.
	SpaceVect clamp(cpFloat len) const;

	/// Returns the angular direction the vector is pointing in (in radians).
	cpFloat toAngle() const;

	/// Uses complex multiplication to return a copy rotated by v. Scaling will occur if the vector isn't a unit vector.
	SpaceVect rotate(SpaceVect v) const;
        
	SpaceVect rotate(cpFloat angleRadians) const;

	/// Inverse of SpaceVect::rotate.
	SpaceVect unrotate(SpaceVect v) const;

	/// x component
	cpFloat x;

	/// y component
	cpFloat y;

    static inline SpaceVect ray(cpFloat radius, cpFloat angle){
        return SpaceVect(cos(angle)*radius,sin(angle)*radius);
    }

	/// Returns the unit length vector for the given angle (in radians).
	static SpaceVect forAngle(cpFloat a);

	/// Vector dot product.
	static cpFloat dot(SpaceVect v1, SpaceVect v2);

	/// 2D vector cross product analog. The cross product of 2D vectors results in a 3D vector with only a z
	/// component. This function returns the value along the z-axis.
	static cpFloat cross(SpaceVect v1, SpaceVect v2);

	/// Returns the vector projection of v1 onto v2.
	static SpaceVect project(SpaceVect v1, SpaceVect v2);

	/// Linearly interpolate between a and b.
	static SpaceVect lerp(SpaceVect v1, SpaceVect v2, cpFloat t);

	/// Linearly interpolate between v1 towards v2 by distance d.
	static SpaceVect lerpconst(SpaceVect v1, SpaceVect v2, cpFloat d);

	/// Spherical linearly interpolate between v1 and v2.
	static SpaceVect slerp(SpaceVect v1, SpaceVect v2, cpFloat t);

	/// Spherical linearly interpolate between v1 towards v2 by no more than angle a in radians.
	static SpaceVect slerpconst(SpaceVect v1, SpaceVect v2, cpFloat a);

	/// Returns the distance between v1 and v2.
	static cpFloat dist(SpaceVect v1, SpaceVect v2);

	/// Returns the squared distance between v1 and v2. Faster than SpaceVect::dist when you only need to compare
	/// distances.
	static cpFloat distSq(SpaceVect v1, SpaceVect v2);
	static bool fuzzyMatch(SpaceVect v1, SpaceVect v2);

    static const SpaceVect zero;
    static const SpaceVect unit_square;
};

bool operator==(const SpaceVect& lhs, const SpaceVect& rhs);
bool operator!=(const SpaceVect& lhs, const SpaceVect& rhs);
SpaceVect operator*(const SpaceVect& lhs, const cpFloat v);
SpaceVect operator/(const SpaceVect& lhs, const cpFloat v);
SpaceVect operator*(const cpFloat v, const SpaceVect& rhs);
SpaceVect operator/(const cpFloat v, const SpaceVect& rhs);
cpFloat operator*(const SpaceVect& lhs, const SpaceVect& rhs);
SpaceVect operator-(const SpaceVect& lhs, const SpaceVect& rhs);
SpaceVect operator+(const SpaceVect& lhs, const SpaceVect& rhs);
bool operator<(const SpaceVect& lhs, const SpaceVect& rhs);
bool operator>(const SpaceVect& lhs, const SpaceVect& rhs);

std::ostream& operator<<(std::ostream&, const SpaceVect&);

#endif //vect_hpp