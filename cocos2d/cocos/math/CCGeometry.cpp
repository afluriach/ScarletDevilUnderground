/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2014 Chukong Technologies

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "math/CCGeometry.h"

#include <algorithm>
#include "base/ccMacros.h"

// implementation of Vec2
NS_CC_BEGIN

// implementation of Size

CCSize::CCSize(void) : width(0), height(0)
{
}

CCSize::CCSize(float w, float h) : width(w), height(h)
{
}

CCSize::CCSize(const CCSize& other) : width(other.width), height(other.height)
{
}

CCSize::CCSize(const Vec2& point) : width(point.x), height(point.y)
{
}

CCSize& CCSize::operator= (const CCSize& other)
{
    setSize(other.width, other.height);
    return *this;
}

CCSize& CCSize::operator= (const Vec2& point)
{
    setSize(point.x, point.y);
    return *this;
}

CCSize CCSize::operator+(const CCSize& right) const
{
    return CCSize(this->width + right.width, this->height + right.height);
}

CCSize CCSize::operator-(const CCSize& right) const
{
    return CCSize(this->width - right.width, this->height - right.height);
}

CCSize CCSize::operator*(float a) const
{
    return CCSize(this->width * a, this->height * a);
}

CCSize CCSize::operator/(float a) const
{
	CCASSERT(a!=0, "CCSize division by 0.");
    return CCSize(this->width / a, this->height / a);
}

void CCSize::setSize(float w, float h)
{
    this->width = w;
    this->height = h;
}

bool CCSize::equals(const CCSize& target) const
{
    return (fabs(this->width  - target.width)  < FLT_EPSILON)
        && (fabs(this->height - target.height) < FLT_EPSILON);
}

const CCSize CCSize::ZERO = CCSize(0, 0);

// implementation of Rect

CCRect::CCRect(void)
{
    setRect(0.0f, 0.0f, 0.0f, 0.0f);
}

CCRect::CCRect(float x, float y, float width, float height)
{
    setRect(x, y, width, height);
}

CCRect::CCRect(const CCRect& other)
{
    setRect(other.origin.x, other.origin.y, other.size.width, other.size.height);
}

CCRect& CCRect::operator= (const CCRect& other)
{
    setRect(other.origin.x, other.origin.y, other.size.width, other.size.height);
    return *this;
}

void CCRect::setRect(float x, float y, float width, float height)
{
    // CGRect can support width<0 or height<0
    // CCASSERT(width >= 0.0f && height >= 0.0f, "width and height of Rect must not less than 0.");

    origin.x = x;
    origin.y = y;

    size.width = width;
    size.height = height;
}

bool CCRect::equals(const CCRect& rect) const
{
    return (origin.equals(rect.origin) && 
            size.equals(rect.size));
}

float CCRect::getMaxX() const
{
    return origin.x + size.width;
}

float CCRect::getMidX() const
{
    return origin.x + size.width / 2.0f;
}

float CCRect::getMinX() const
{
    return origin.x;
}

float CCRect::getMaxY() const
{
    return origin.y + size.height;
}

float CCRect::getMidY() const
{
    return origin.y + size.height / 2.0f;
}

float CCRect::getMinY() const
{
    return origin.y;
}

float CCRect::getWidth() const
{
	return size.width;
}

float CCRect::getHeight() const
{
	return size.height;
}

bool CCRect::containsPoint(const Vec2& point) const
{
    bool bRet = false;

    if (point.x >= getMinX() && point.x <= getMaxX()
        && point.y >= getMinY() && point.y <= getMaxY())
    {
        bRet = true;
    }

    return bRet;
}

bool CCRect::intersectsRect(const CCRect& rect) const
{
    return !(     getMaxX() < rect.getMinX() ||
             rect.getMaxX() <      getMinX() ||
                  getMaxY() < rect.getMinY() ||
             rect.getMaxY() <      getMinY());
}

bool CCRect::intersectsCircle(const cocos2d::Vec2 &center, float radius) const
{
    Vec2 rectangleCenter((origin.x + size.width / 2),
                         (origin.y + size.height / 2));
    
    float w = size.width / 2;
    float h = size.height / 2;
    
    float dx = fabs(center.x - rectangleCenter.x);
    float dy = fabs(center.y - rectangleCenter.y);
    
    if (dx > (radius + w) || dy > (radius + h))
    {
        return false;
    }
    
    Vec2 circleDistance(fabs(center.x - origin.x - w),
                        fabs(center.y - origin.y - h));
    
    if (circleDistance.x <= (w))
    {
        return true;
    }
    
    if (circleDistance.y <= (h))
    {
        return true;
    }
    
    float cornerDistanceSq = powf(circleDistance.x - w, 2) + powf(circleDistance.y - h, 2);
    
    return (cornerDistanceSq <= (powf(radius, 2)));
}

void CCRect::merge(const CCRect& rect)
{
    float top1    = getMaxY();
    float left1   = getMinX();
    float right1  = getMaxX();
    float bottom1 = getMinY();
    
    float top2    = rect.getMaxY();
    float left2   = rect.getMinX();
    float right2  = rect.getMaxX();
    float bottom2 = rect.getMinY();
    origin.x = std::min(left1, left2);
    origin.y = std::min(bottom1, bottom2);
    size.width = std::max(right1, right2) - origin.x;
    size.height = std::max(top1, top2) - origin.y;
}

CCRect CCRect::unionWithRect(const CCRect & rect) const
{
    float thisLeftX = origin.x;
    float thisRightX = origin.x + size.width;
    float thisTopY = origin.y + size.height;
    float thisBottomY = origin.y;
    
    if (thisRightX < thisLeftX)
    {
        std::swap(thisRightX, thisLeftX);   // This rect has negative width
    }
    
    if (thisTopY < thisBottomY)
    {
        std::swap(thisTopY, thisBottomY);   // This rect has negative height
    }
    
    float otherLeftX = rect.origin.x;
    float otherRightX = rect.origin.x + rect.size.width;
    float otherTopY = rect.origin.y + rect.size.height;
    float otherBottomY = rect.origin.y;
    
    if (otherRightX < otherLeftX)
    {
        std::swap(otherRightX, otherLeftX);   // Other rect has negative width
    }
    
    if (otherTopY < otherBottomY)
    {
        std::swap(otherTopY, otherBottomY);   // Other rect has negative height
    }
    
    float combinedLeftX = std::min(thisLeftX, otherLeftX);
    float combinedRightX = std::max(thisRightX, otherRightX);
    float combinedTopY = std::max(thisTopY, otherTopY);
    float combinedBottomY = std::min(thisBottomY, otherBottomY);
    
    return CCRect(combinedLeftX, combinedBottomY, combinedRightX - combinedLeftX, combinedTopY - combinedBottomY);
}

const CCRect CCRect::ZERO = CCRect(0, 0, 0, 0);

NS_CC_END
