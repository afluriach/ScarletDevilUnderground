//
//  FileIO.hpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#ifndef FileIO_hpp
#define FileIO_hpp

class ControlReplay;
class GState;

namespace io
{
	
string pathString(const boost::filesystem::path& p);
string removeExtension(const string& filename);
set<string> getFileNamesInFolder(const string& filepath);
set<string> getProfiles();
set<string> getReplays();

void checkCreateSubfolders();
string getProfilePath();
string getReplayFolderPath();

unique_ptr<GState> loadProfileState(string name);
bool saveProfileState(const GState* state, string path);

unique_ptr<ControlReplay> getControlReplay(string name);
void autosaveControlReplay(string sceneName, const ControlReplay* cr);
void saveControlReplay(string name, const ControlReplay* cr);
}

#endif /* FileIO_hpp */
