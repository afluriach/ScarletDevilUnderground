//
//  Resources.hpp
//  Koumachika
//
//  Created by Toni on 4/13/19.
//
//

#ifndef Resources_hpp
#define Resources_hpp

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	#include "../platform/win32/CCFileUtils-win32.h"
	#define FileUtilsImpl FileUtilsWin32
#endif

class FileUtilsZip : public FileUtilsImpl
{
public:
	typedef pair<unsigned char*, ssize_t> zip_file;

	inline FileUtilsZip() {}
	virtual ~FileUtilsZip();

	virtual bool init();

	virtual string fullPathForFilename(const string& filename) const;
	virtual Data getDataFromFile(const string& filename);
	virtual string getStringFromFile(const string& filename);
	virtual bool isFileExist(const std::string& filename) const;

protected:
	bool loadFileData(const string& filename);
	void unloadAll();

	const unsigned char* getData(const string& filename);
	const char* getDataBytes(const string& filename);
	Data makeDataCopy(const string& filename);

	//ZipFile returns a pointer to a malloc-allocated buffer.
	unordered_map<string, zip_file> loadedFiles;
	unique_ptr<ZipFile> zipFile;
};

#endif /* Resources_hpp */
