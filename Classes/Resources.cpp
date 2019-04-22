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

#define auto_this auto _this = static_cast<libsnd_file_pointer*>(_this_);

sf_count_t libsnd_file_pointer::file_length(void* _this_)
{
	auto_this
	return _this->file_buf->second;
}

sf_count_t libsnd_file_pointer::seek(sf_count_t _offset, int whence, void* _this_)
{
	auto_this

	if (whence == SEEK_SET) {
		_this->offset = _offset;
	}
	else if (whence == SEEK_CUR) {
		_this->offset += _offset;
	}
	else if (whence == SEEK_END) {
		_this->offset = _this->file_buf->second + _offset;
	}

	return _this->offset;
}

sf_count_t libsnd_file_pointer::read(void* output_buf, sf_count_t count, void* _this_)
{
	auto_this
	sf_count_t bytes_remaining = _this->file_buf->second - _this->offset;
	sf_count_t actual = count <= bytes_remaining ? count : bytes_remaining;
	unsigned char* start = _this->file_buf->first + _this->offset;
	auto output = static_cast<unsigned char*>(output_buf);

	copy(start,start + actual,output);
	_this->offset += actual;
	return actual;
}

sf_count_t libsnd_file_pointer::tell(void* _this_)
{
	auto_this
	return _this->offset;
}

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

string FileUtilsZip::fullPathForFilename(const string &filename) const
{
	string _fp = convertFilepath(filename);
	if (zipFile && zipFile->fileExists(_fp)) return _fp;
	else return FileUtilsImpl::fullPathForFilename(filename);
}

Data FileUtilsZip::getDataFromFile(const string& filename)
{
	if (zipFile && zipFile->fileExists(filename)) {
		loadFileData(filename);
		return move(makeDataCopy(filename));
	}
	else{
		return FileUtilsImpl::getDataFromFile(filename);
	}
}

string FileUtilsZip::getStringFromFile(const string& filename)
{
	if (zipFile && zipFile->fileExists(filename) && loadFileData(filename)) {
		const zip_file& entry = loadedFiles.at(filename);
		return string((const char*)entry.first, entry.second);
	}
	else return FileUtilsImpl::getStringFromFile(filename);
}

bool FileUtilsZip::isFileExist(const string& filename) const
{
	if (zipFile && zipFile->fileExists(filename)) return true;
	else return FileUtilsImpl::isFileExist(filename);
}

SNDFILE* FileUtilsZip::openSoundFile(const string& filename, SF_INFO* info)
{
	if (!loadFileData(filename)) {
		log("openSoundFile: %s not found in resources.zip", filename.c_str());
		return nullptr;
	}

	SF_VIRTUAL_IO callbacks = {
		&libsnd_file_pointer::file_length,
		&libsnd_file_pointer::seek,
		&libsnd_file_pointer::read,
		nullptr,
		&libsnd_file_pointer::tell,
	};

	unique_ptr<libsnd_file_pointer> fp = make_unique<libsnd_file_pointer>(libsnd_file_pointer{0, &loadedFiles.at(filename)});
	soundFileHandles.insert_or_assign(filename, move(fp));
	return sf_open_virtual(&callbacks, SFM_READ, info, soundFileHandles.at(filename).get());
}

void FileUtilsZip::closeSoundFile(const string& filename)
{
	soundFileHandles.erase(filename);
}

void FileUtilsZip::unloadFile(const string& filename)
{
	auto it = loadedFiles.find(filename);
	if (it != loadedFiles.end()) {
		free(it->second.first);
		loadedFiles.erase(it);
	}
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
