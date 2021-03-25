//
//  FileIO.hpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#ifndef FileIO_hpp
#define FileIO_hpp

class GState;

namespace io
{
	
string pathString(const boost::filesystem::path& p);
string removeExtension(const string& filename);
set<string> getFileNamesInFolder(const string& filepath);
set<string> getProfiles();

string loadTextFile(const string& res);

void checkCreateSubfolders();
string getWriteablePath();
string getProfilePath();
string getControlMappingPath();
string getConfigFilePath();
string getRealPath(const string& path);

string profileName(unsigned long idx);
bool deleteProfile(string name);
unique_ptr<GState> loadProfileState(string name);
bool saveProfileState(const GState* state, string path);

}

#endif /* FileIO_hpp */
