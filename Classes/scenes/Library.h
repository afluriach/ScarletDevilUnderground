//
//  Library.h
//  FlansBasement
//
//  Created by Toni on 11/27/15.
//
//

#ifndef Library_h
#define Library_h

class Library : public PlayScene
{
public:
    inline Library() : PlayScene("maps/library.tmx"){
    }
    
    CREATE_FUNC(Library);
};

#endif /* Library_h */
