#pragma once

#include "shape.hpp"
#include "body.hpp"

#include <vector>

#ifndef _MSC_VER
#pragma GCC visibility push(default)
#endif
namespace cp {
	/// Convex polygon shape
	class PolyShape : public Shape {
	public:
		PolyShape(std::shared_ptr<Body>, const std::vector<Vect>& verts, Vect offset = Vect());
		int getNumVerts() const;
		cp::Vect getVert(int);
        
        static inline std::shared_ptr<PolyShape> rectangle(std::shared_ptr<Body> body, const Vect& dim)
        {
            Float hw = dim.x / 2;
            Float hh = dim.y / 2;
            
            Float l = -hw;
            Float r = hw;
            Float b = -hh;
            Float t = hh;
            
            std::vector<Vect> verts = {
                Vect(l,b),
                Vect(l,t),
                Vect(r,t),
                Vect(r,b)
            };
            
            return std::make_shared<PolyShape>(body,verts);
        }
	};
}
#ifndef _MSC_VER
#pragma GCC visibility pop
#endif
