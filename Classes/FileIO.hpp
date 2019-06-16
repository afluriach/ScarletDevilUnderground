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
class Replay;

namespace io
{
	
string pathString(const boost::filesystem::path& p);
string removeExtension(const string& filename);
set<string> getFileNamesInFolder(const string& filepath);
set<string> getProfiles();
set<string> getReplays();

string loadTextFile(const string& res);

void checkCreateSubfolders();
string getProfilePath();
string getReplayFolderPath();
string getControlMappingPath();
string getConfigFilePath();
string getRealPath(const string& path);

unique_ptr<GState> loadProfileState(string name);
bool saveProfileState(const GState* state, string path);

unique_ptr<Replay> getControlReplay(string name);
void autosaveControlReplay(string sceneName, const Replay* cr);
void saveControlReplay(string name, const Replay* cr);
}

#endif /* FileIO_hpp */
