#include "body.hpp"

namespace cp {
	Body::Body(cpFloat mass, cpFloat inertia) : body(cpBodyNew(mass, inertia)), owning(true) {
	}

	Body::Body(Body&& other) : body(other.body), owning(other.owning) {
		other.body = nullptr;
		other.owning = false;
	}

	Body::~Body() {
		if (owning) {
			cpBodyFree(body);
		}
	}

	Body::Body(cpBody* body) : body(body), owning(false) {
	}

	Body::operator cpBody*() const {
		return body;
	}

	void Body::setPos(Vect p) {
		cpBodySetPos(body, p);
	}

	Vect Body::getPos() const {
		return cpBodyGetPos(body);
	}

	void Body::setAngle(float angle){
	    body->a = angle;
	    body->rot = cpvforangle(angle);
	    cpBodyAssertSane(body);
	}
	
	float Body::getAngle() const{
	    return body->a;
	}

	Vect Body::getVel() const {
		return cpBodyGetVel(body);
	}

	void Body::setVel(Vect velocity) {
		cpBodySetVel(body, velocity);
	}
    
    float Body::getAngularVel() const{
        return body->w;
    }
    
    void Body::setAngularVel(float w){
        body->w = w;
    }

	Layers Body::getLayers()
	{
		unsigned int result = 0;

		for (auto it = shapes.begin(); it != shapes.end(); ++it)
			result |= static_cast<unsigned int>((*it)->getLayers());

		return result;
	}

	void Body::setAllLayers(Layers layers)
	{
		for (auto it = shapes.begin(); it != shapes.end(); ++it)
			(*it)->setLayers(layers);
	}

    void Body::applyImpulse(Vect force){
        cpBodyApplyImpulse(body,force,Vect(0,0));
    }

	Float Body::getVelLimit() const {
		return cpBodyGetVelLimit(body);
	}

	void Body::setVelLimit(Float limit) {
		cpBodySetVelLimit(body, limit);
	}

	DataPointer Body::getUserData() const {
		return cpBodyGetUserData(body);
	}

	void Body::setUserData(DataPointer p) {
		cpBodySetUserData(body, p);
	}
    void Body::removeShapes(Space& space){
        for(auto it=shapes.begin(); it != shapes.end(); ++it){
            space.remove(*it);
        }
    }

}
