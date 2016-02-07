//
//  Clearing.hpp
//  Koumachika
//
//  Created by Toni on 2/6/16.
//
//

#ifndef Clearing_h
#define Clearing_h

class Clearing : public PlayScene
{
public:
    CREATE_FUNC(Clearing);
    inline Clearing() : PlayScene("maps/clearing.tmx"){
    }
};

#endif /* Clearing_h */
