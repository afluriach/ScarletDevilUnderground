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

using namespace boost::filesystem;

namespace io
{

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

			log("%s", s.c_str());
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

unique_ptr<GState> loadProfileState(string name)
{
	string profilePath = io::getProfilePath() + name + ".profile";
	unique_ptr<GState> result = make_unique<GState>();

	if (!FileUtils::getInstance()->isFileExist(profilePath))
	{
		log("Profile %s does not exist!", name.c_str());
		return nullptr;
	}
	else
	{
		try {
			ifstream ifs(profilePath);
			boost::archive::binary_iarchive ia(ifs);
			ia >> *result;
			log("Profile %s loaded.", profilePath.c_str());
			return result;
		}
		catch (boost::archive::archive_exception e) {
			log("Error while loading: %s", e.what());
			return nullptr;
		}
	}
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

void autosaveControlReplay(string sceneName, ControlReplay* cr)
{
	int idx = 1;

	do
	{
		string filepath = io::getReplayFolderPath() + sceneName + boost::lexical_cast<string>(idx) + ".replay";
		bool exists = FileUtils::getInstance()->isFileExist(filepath);

		if (!exists) {
			ofstream ofs(filepath);
			boost::archive::binary_oarchive oa(ofs);
			oa << *cr;
			log("Replay %s saved.", filepath.c_str());
			break;
		}
		++idx;
	}
	while(true);
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