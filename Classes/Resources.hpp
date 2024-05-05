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

#ifdef CC_TARGET_OS_MAC
	#include "../platform/apple/CCFileUtils-apple.h"
	#define FileUtilsImpl FileUtilsApple
#endif

typedef pair<unsigned char*, ssize_t> zip_file;

struct libsnd_file_pointer
{
	static sf_count_t file_length(void* _this_);
	static sf_count_t seek(sf_count_t _offset, int whence, void* _this_);
	static sf_count_t read(void* output_buf, sf_count_t count, void* _this_);
	static sf_count_t tell(void* _this_);

	sf_count_t offset = 0;
	const zip_file* file_buf = nullptr;
};

class FileUtilsZip : public FileUtilsImpl
{
public:
	static string convertFilepath(const string& path);

	inline FileUtilsZip() {}
	virtual ~FileUtilsZip();

	virtual bool init();

	virtual string fullPathForFilename(const string& filename) const;
	virtual Data getDataFromFile(const string& filename);
	virtual string getStringFromFile(const string& filename);
	virtual bool isFileExist(const string& filename) const;

#if use_sound
	SNDFILE* openSoundFile(const string& filename, SF_INFO* info);
	void closeSoundFile(const string& filename);
#endif

	void unloadFile(const string& filename);
protected:
	bool loadFileData(const string& filename);
	void unloadAll();

	const unsigned char* getData(const string& filename);
	const char* getDataBytes(const string& filename);
	Data makeDataCopy(const string& filename);

	//ZipFile returns a pointer to a malloc-allocated buffer.
	unordered_map<string, zip_file> loadedFiles;
	unique_ptr<ZipFile> zipFile;
	unordered_map<string, unique_ptr<libsnd_file_pointer>> soundFileHandles;
};

#endif /* Resources_hpp */
