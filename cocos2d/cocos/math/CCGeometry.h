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

#ifndef __MATH_CCGEOMETRY_H__
#define __MATH_CCGEOMETRY_H__

#include <math.h>

#include "platform/CCPlatformMacros.h"
#include "base/ccMacros.h"
#include "math/CCMath.h"

/**
 * @addtogroup base
 * @{
 */

NS_CC_BEGIN

class CC_DLL CCSize
{
public:
    /**Width of the Size.*/
    float width;
    /**Height of the Size.*/
    float height;
public:
    /**Conversion from Vec2 to Size.*/
    operator Vec2() const
    {
        return Vec2(width, height);
    }

public:
    /**
    @{
    Constructor.
    @param width Width of the size.
    @param height Height of the size.
    @param other Copy constructor.
    @param point Conversion from a point.
     */
    CCSize();
    CCSize(float width, float height);
    CCSize(const CCSize& other);
    explicit CCSize(const Vec2& point);
    /**@}*/

    /**
     * @js NA
     * @lua NA
     */
    CCSize& operator= (const CCSize& other);
    /**
     * @js NA
     * @lua NA
     */
    CCSize& operator= (const Vec2& point);
    /**
     * @js NA
     * @lua NA
     */
    CCSize operator+(const CCSize& right) const;
    /**
     * @js NA
     * @lua NA
     */
    CCSize operator-(const CCSize& right) const;
    /**
     * @js NA
     * @lua NA
     */
    CCSize operator*(float a) const;
    /**
     * @js NA
     * @lua NA
     */
    CCSize operator/(float a) const;
    /**
    Set the width and height of Size.
     * @js NA
     * @lua NA
     */
    void setSize(float width, float height);
    /**
    Check if two size is the same.
     * @js NA
     */
    bool equals(const CCSize& target) const;
    /**Size(0,0).*/
    static const CCSize ZERO;
    
    //Return the larger dimension
    inline float getBoundingDimension() const{
        return (width > height) ? width : height;
    }
};

/**Rectangle area.*/
class CC_DLL CCRect
{
public:
    /**Low left point of rect.*/
    Vec2 origin;
    /**Width and height of the rect.*/
    CCSize  size;

public:
    /**
    Constructor an empty Rect.
     * @js NA
     */
    CCRect();
    /**
    Constructor a rect.
     * @js NA
     */
    CCRect(float x, float y, float width, float height);
    /**
    Copy constructor.
     * @js NA
     * @lua NA
     */
    CCRect(const CCRect& other);
    /**
     * @js NA
     * @lua NA
     */
    CCRect& operator= (const CCRect& other);
    /**
    Set the x, y, width and height of Rect.
     * @js NA
     * @lua NA
     */
    void setRect(float x, float y, float width, float height);
    /**
    Get the left of the rect.
     * @js NA
     */
    float getMinX() const; /// return the leftmost x-value of current rect
    /**
    Get the X coordinate of center point.
     * @js NA
     */
    float getMidX() const; /// return the midpoint x-value of current rect
    /**
    Get the right of rect.
     * @js NA
     */
    float getMaxX() const; /// return the rightmost x-value of current rect
    /**
    Get the bottom of rect.
     * @js NA
     */
    float getMinY() const; /// return the bottommost y-value of current rect
    /**
    Get the Y coordinate of center point.
     * @js NA
     */
    float getMidY() const; /// return the midpoint y-value of current rect
    /**
    Get top of rect.
     * @js NA
     */
    float getMaxY() const; /// return the topmost y-value of current rect
    /**
    Compare two rects.
     * @js NA
     */
    bool equals(const CCRect& rect) const;
    /**
    Check if the points is contained in the rect.
     * @js NA
     */
    bool containsPoint(const Vec2& point) const;
    /**
    Check the intersect status of two rects.
     * @js NA
     */
    bool intersectsRect(const CCRect& rect) const;
    /**
    Check the intersect status of the rect and a circle.
     * @js NA
     */
    bool intersectsCircle(const Vec2& center, float radius) const;
    /**
    Get the min rect which can contain this and rect.
     * @js NA
     * @lua NA
     */
    CCRect unionWithRect(const CCRect & rect) const;
    /**Compute the min rect which can contain this and rect, assign it to this.*/
    void merge(const CCRect& rect);
    /**An empty Rect.*/
    static const CCRect ZERO;
    
    inline Vec2 getUpperCorner() const {
        return Vec2(origin.x+size.width, origin.y + size.height);
    }
};

NS_CC_END

// end of base group
/// @}

#endif // __MATH_CCGEOMETRY_H__
