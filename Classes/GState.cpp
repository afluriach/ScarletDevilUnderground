//
//  GState.cpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#include "Prefix.h"

#include "GState.hpp"

GState GState::crntState;

void GState::load()
{
    string profilePath = getProfilePath();
    if(!FileUtils::getInstance()->isFileExist(profilePath))
    {
        log("Profile does not exist, creating a new one");
    }
    else
    {
        try{
            ifstream ifs(profilePath);
            boost::archive::binary_iarchive ia(ifs);
            ia >> crntState;
            log("Profile %s loaded.", profilePath.c_str());
        }
        catch(boost::archive::archive_exception e){
            log("Error while loading: %s", e.what());
        }
    }
}

void GState::save()
{
    checkCreateProfileSubfolder();
    string profilePath = getProfilePath();
    
    try{
        
        ofstream ofs(profilePath);
        boost::archive::binary_oarchive oa(ofs);
        oa << crntState;
        log("Profile %s saved.", profilePath.c_str());
    }
    catch(boost::archive::archive_exception e){
        log("Error while saving: %s", e.what());
    }
    
}
