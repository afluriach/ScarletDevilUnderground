//
//  Library.hpp
//  Koumachika
//
//  Created by Toni on 1/22/19.
//
//

#ifndef Library_hpp
#define Library_hpp

#include "PlayScene.hpp"
#include "types.h"

class Library : public PlayScene
{
public:
	Library();

	inline virtual Color3B getDefaultAmbientLight() const { return toColor3B(Color4F(0.5f, 0.5f, 0.2f, 1.0f)); }
	virtual inline GScene* getReplacementScene() { return Node::ccCreate<Library>(); }
};

#endif /* Library_hpp */
