//
//  log.hpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#ifndef log_hpp
#define log_hpp

class LogSystem
{
public:
	static void logOutput(const string& s);

	static void initThread();
	static void exit();
protected:
	static void update();

	static mutex bufMutex;
	static vector<string> buf;
	static ofstream outputFile;

	static thread logThread;
	static condition_variable threadCondVar;
	static mutex threadCondMutex;
	static atomic_bool exitFlag;
};

#define debug_log(s) log("%s, %d: %s", __FUNCTION__, __LINE__, s);

template<typename... T>
void log_print(string s, T... args)
{
	boost::format fmt(s);
	string result = boost::str((fmt % ... % forward<T>(args)));

	LogSystem::logOutput(result);
}

#define log log_print

#endif
