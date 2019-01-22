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

	virtual inline GScene* getReplacementScene() { return Node::ccCreate<Library>(); }
};

#endif /* Library_hpp */
