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

namespace io
{
	
string pathString(const boost::filesystem::path& p);
string removeExtension(const string& filename);
set<string> getProfiles();
set<string> getReplays();

void checkCreateSubfolders();
string getProfilePath();
string getReplayFolderPath();

unique_ptr<ControlReplay> getControlReplay(string name);
void autosaveControlReplay(string sceneName, ControlReplay* cr);
void saveControlReplay(string name, ControlReplay* cr);
}

#endif /* FileIO_hpp */
