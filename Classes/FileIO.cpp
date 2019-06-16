//
//  FileIO.cpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "controls.h"
#include "FileIO.hpp"
#include "GState.hpp"
#include "replay.h"
#include "util.h"

using namespace boost::filesystem;

namespace io
{

template<typename T>
unique_ptr<T> loadData(string path)
{
	unique_ptr<T> result = make_unique<T>();
	const char* cpath = path.c_str();

	if (!FileUtils::getInstance()->isFileExist(path))
	{
		log("File %s does not exist!", cpath);
		return nullptr;
	}
	else
	{
		try {
			std::ifstream ifs(path);

			if (!ifs) {
				log("Failed to open input stream for %s.", cpath);
				return nullptr;
			}

			boost::archive::binary_iarchive ia(ifs);
			ia >> *result;
			log("File %s loaded.", cpath);
			return result;
		}
		catch (boost::archive::archive_exception e) {
			log("archive/serialize error code %d while loading file %s", e.code, cpath);
			return nullptr;
		}
	}
}

template<typename T>
bool saveData(const T* data, string path, bool overwrite)
{
	const char* cpath = path.c_str();
	bool exists = FileUtils::getInstance()->isFileExist(path);

	if (exists && !overwrite)
	{
		log("File %s exists, not overwriting!", cpath);
		return false;
	}
	else
	{
		try {
			std::ofstream ofs(path);

			if (!ofs) {
				log("Failed to open output stream for %s.", cpath);
				return nullptr;
			}


			boost::archive::binary_oarchive oa(ofs);
			oa << *data;

			if (exists) {
				log("File %s overwritten.", cpath);
			}
			else {
				log("File %s saved.", cpath);
			}
			return true;
		}
		catch (boost::archive::archive_exception e) {
			log("archive/serialize error code %d while saving file %s", e.code, cpath);
			return false;
		}
	}
}

string pathString(const boost::filesystem::path& p)
{
	stringstream ss;
	ss << p;
	return ss.str();
}

string removeExtension(const string& filename)
{
	size_t dotIdx = filename.find_last_of(".");
	
	return dotIdx != string::npos ? filename.substr(0, dotIdx) : filename;
}

set<string> getFileNamesInFolder(const string& filepath)
{
	set<string> result;
	path directory(filepath);

	try
	{
		if (!exists(directory)) {
			log("Directory %s not found!", pathString(directory).c_str());
			return result;
		}

		if (!is_directory(directory)) {
			log("%s is not a directory!", pathString(directory).c_str());
			return result;
		}

		for (directory_entry& entry : directory_iterator(directory))
		{
			string s = entry.path().filename().generic_string();
			result.insert(removeExtension(s));
		}
	}
	catch (const filesystem_error& ex) {
		log("Filesystem error: %s", ex.what());
	}

	return result;
}

set<string> getProfiles()
{
	return getFileNamesInFolder(getProfilePath());
}

set<string> getReplays()
{
	return getFileNamesInFolder(getReplayFolderPath());
}

string loadTextFile(const string& res)
{
	return FileUtils::getInstance()->getStringFromFile(res);
}

void checkCreateSubfolders()
{
	auto* f = FileUtils::getInstance();

	if (!f->isDirectoryExist(getProfilePath())) {
		log("profiles/ folder created.");
		f->createDirectory(getProfilePath());
	}

	if (!f->isDirectoryExist(getReplayFolderPath())) {
		f->createDirectory(getReplayFolderPath());
		log("replays/ folder created.");
	}
}

string getProfilePath()
{
	return App::getBaseDataPath() + "profiles/";
}

string getReplayFolderPath()
{
	return App::getBaseDataPath() + "replays/";
}

string getControlMappingPath()
{
	return App::getBaseDataPath() + "control_mapping.txt";
}

string getConfigFilePath()
{
	return App::getBaseDataPath() + "config.txt";
}

string getRealPath(const string& path)
{
	return FileUtils::getInstance()->fullPathForFilename(path);
}

unique_ptr<GState> loadProfileState(string name)
{
	string profilePath = io::getProfilePath() + name + ".profile";
	return loadData<GState>(profilePath);
}
bool saveProfileState(const GState* state, string name)
{
	string profilePath = io::getProfilePath() + name + ".profile";
	return saveData<GState>(state, profilePath, true);
}

unique_ptr<Replay> getControlReplay(string name)
{
	string filepath = io::getReplayFolderPath() + name + ".replay";
	return loadData<Replay>(filepath);
}

void autosaveControlReplay(string sceneName, const Replay* cr)
{
	string filepath = io::getReplayFolderPath() + sceneName + " " + getNowTimestamp() + ".replay";
	saveData<Replay>(cr, filepath, false);
}

void saveControlReplay(string name, const Replay* cr)
{
	string filepath = io::getReplayFolderPath() + name + ".replay";
	saveData<Replay>(cr, filepath, false);
}

}
