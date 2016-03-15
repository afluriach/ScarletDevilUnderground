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

class ClearingPath : public PlayScene
{
public:
    CREATE_FUNC(ClearingPath);
    inline ClearingPath() : PlayScene("maps/clearing_path.tmx"){
    }
};


#endif /* Clearing_h */
