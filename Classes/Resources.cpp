//
//  Resources.cpp
//  Koumachika
//
//  Created by Toni on 4/13/19.
//
//

#include "Prefix.h"

#include "macros.h"
#include "Resources.hpp"
#include "util.h"

string FileUtilsZip::convertFilepath(const string& path)
{
	vector<string> tokens = splitString(path, "/\\");
	vector<string> filteredTokens;
	string result;

	for (string token : tokens)
	{
		if (token == ".") continue;
		else if (token == ".." && !filteredTokens.empty()) {
			filteredTokens.pop_back();
		}
		else{
			filteredTokens.push_back(token);
		}
	}

	if (filteredTokens.empty()) {
		return string();
	}

	result += filteredTokens.at(0);

	for_irange(i, 1, filteredTokens.size()) {
		result += "/" + filteredTokens.at(i);
	}
	return result;
}

FileUtilsZip::~FileUtilsZip()
{
	unloadAll();
}

bool FileUtilsZip::init()
{
	FileUtils::init();

	zipFile = make_unique<ZipFile>("resources.zip");

	return to_bool(zipFile);
}

string FileUtilsZip::fullPathForFilename(const std::string &filename) const
{
	string _fp = convertFilepath(filename);
	if (zipFile && zipFile->fileExists(_fp)) return _fp;
	else return FileUtilsImpl::fullPathForFilename(filename);
}

Data FileUtilsZip::getDataFromFile(const std::string& filename)
{
	if (zipFile && zipFile->fileExists(filename)) {
		loadFileData(filename);
		return move(makeDataCopy(filename));
	}
	else{
		return FileUtilsImpl::getDataFromFile(filename);
	}
}

string FileUtilsZip::getStringFromFile(const std::string& filename)
{
	if (zipFile && zipFile->fileExists(filename) && loadFileData(filename)) {
		const zip_file& entry = loadedFiles.at(filename);
		return string((const char*)entry.first, entry.second);
	}
	else return FileUtilsImpl::getStringFromFile(filename);
}

bool FileUtilsZip::isFileExist(const std::string& filename) const
{
	if (zipFile && zipFile->fileExists(filename)) return true;
	else return FileUtilsImpl::isFileExist(filename);
}

bool FileUtilsZip::loadFileData(const string& filename)
{
	auto it = loadedFiles.find(filename);
	if (it != loadedFiles.end()) {
		log("File %s already loaded from resources.zip", filename.c_str());
		return true;
	}

	if (!zipFile || !zipFile->fileExists(filename)) {
		log("File %s not found in resources.zip", filename.c_str());
		return false;
	}

	ssize_t filesize = 0;
	unsigned char* buf = zipFile->getFileData(filename, &filesize);
	if (buf == nullptr || filesize == 0) {
		log("Failed to load file %s from resources.zip", filename.c_str());
		return false;
	}

	loadedFiles.insert_or_assign(filename, make_pair(buf, filesize));
	return true;
}

void FileUtilsZip::unloadAll()
{
	for (auto entry : loadedFiles) {
		free(entry.second.first);
	}
	loadedFiles.clear();
}

const unsigned char* FileUtilsZip::getData(const string& filename)
{
	loadFileData(filename);

	auto it = loadedFiles.find(filename);
	if (it != loadedFiles.cend()) {
		return it->second.first;
	}
	else {
		return nullptr;
	}
}

const char* FileUtilsZip::getDataBytes(const string& filename)
{
	auto it = loadedFiles.find(filename);
	return it != loadedFiles.end() ? (const char*)it->second.second : nullptr;
}

Data FileUtilsZip::makeDataCopy(const string& filename)
{
	auto it = loadedFiles.find(filename);
	if (it == loadedFiles.end())
		return Data::Null;

	Data result;
	result.copy(it->second.first, it->second.second);
	return move(result);
}
