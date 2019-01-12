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

using namespace boost::filesystem;

namespace io
{

string pathString(const boost::filesystem::path& p)
{
	stringstream ss;
	ss << p;
	return ss.str();
}

set<string> getProfiles()
{
	set<string> result;
	path profileDir(getProfilePath());

	log("getProfiles(): %s", pathString(profileDir).c_str());

	try
	{
		if (!exists(profileDir)) {
			log("Profile directory %s not found!", pathString(profileDir).c_str());
			return result;
		}

		if (!is_directory(profileDir)) {
			log("%s is not a directory!", pathString(profileDir).c_str());
			return result;
		}

		for (directory_entry& entry : directory_iterator(profileDir))
		{
			string s = pathString(entry.path());
			log("%s", s.c_str());
			result.insert(s);
		}
	}
	catch (const filesystem_error& ex) {
		log("Filesystem error: %s", ex.what());
	}

	return result;
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

unique_ptr<ControlReplay> getControlReplay(string name)
{
	string filepath = io::getReplayFolderPath() + name + ".replay";
	unique_ptr<ControlReplay> result = make_unique<ControlReplay>();

	if (!FileUtils::getInstance()->isFileExist(filepath)) {
		log("Replay %s not found!", name.c_str());
		return nullptr;
	}

	try {
		ifstream ifs(filepath);
		boost::archive::binary_iarchive ia(ifs);
		ia >> *result;
		log("Replay %s loaded.", name.c_str());
		return result;
	}
	catch (boost::archive::archive_exception e) {
		log("Archive exception loading replay: %s", e.what());
		return nullptr;
	}
}

void saveControlReplay(string name, ControlReplay* cr)
{
	string filepath = io::getReplayFolderPath() + name + ".replay";
	bool exists = FileUtils::getInstance()->isFileExist(filepath);

	try {
		ofstream ofs(filepath);
		boost::archive::binary_oarchive oa(ofs);
		oa << *cr;

		if (exists) {
			log("Replay %s overwritten.", name.c_str());
		}
		else {
			log("Replay %s saved.", name.c_str());
		}
	}
	catch (boost::archive::archive_exception e) {
		log("Error while saving Replay: %s", e.what());
	}
}

}