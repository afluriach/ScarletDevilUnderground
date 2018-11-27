#pragma once

#include "vect.hpp"
#include "space.hpp"

#include <chipmunk.h>

#ifndef _MSC_VER
#pragma GCC visibility push(default)
#endif
namespace cp {
	/**
	 * \brief Rigid body
	 *
	 * A rigid body holds the physical properties of an object. (mass, position, rotation, velocity, etc.) It does not
	 * have a shape until you attach one or more collision shapes to it. If you’ve done physics with particles before,
	 * rigid bodies differ in that they are able to rotate. Rigid bodies generally tend to have a 1:1 correlation to
	 * sprites in a game. You should structure your game so that you use the position and rotation of the rigid body
	 * for drawing your sprite.
	 **/
	class Body {
	public:
		Body(Float mass, Float inertia);
		Body(Body&&);
		explicit Body(cpBody*);
		~Body();
		operator cpBody*() const;
		Vect getPos() const;
		void setPos(Vect);
		
		void setAngle(float);
		float getAngle() const;

		/// Linear velocity of the center of gravity of the body.
		Vect getVel() const;
		void setVel(Vect velocity);
        
        float getAngularVel() const;
        void setAngularVel(float);

		void setAllLayers(Layers layers);

		/// Velocity limit of the body. Defaults to INFINITY unless you set it specifically. Can be used to limit
		/// falling speeds, etc.
		Float getVelLimit() const;
		void setVelLimit(Float limit);

		DataPointer getUserData() const;
		void setUserData(DataPointer);
        
        inline void addShape(std::shared_ptr<Shape> shape){
            shapes.push_back(shape);
        }
        
        inline float getMass() const {
            return body->m;
        }
        
        void applyImpulse(Vect force);

        
        void remove(Space& space);
        void removeShapes(Space& space);
        
	private:
        std::vector<std::shared_ptr<Shape>> shapes;
		Body(const Body&);
		const Body& operator=(const Body&);
		cpBody* body;
		bool owning;
	};
}
#ifndef _MSC_VER
#pragma GCC visibility pop
#endif
