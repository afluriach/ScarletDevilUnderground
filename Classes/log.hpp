//
//  log.hpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#ifndef log_hpp
#define log_hpp

static constexpr bool logAI = true;
static constexpr bool logPathfinding = true;
static constexpr bool logSpells = false;

class LogSystem
{
public:
	static void logOutput(const string& prefix, const string& s);

	static void initThread();
	static void exit();
protected:
	static void update();
 
    static string prevDebugPrefix;

	static mutex bufMutex;
	static vector<string> buf;
	static ofstream outputFile;

	static thread logThread;
	static condition_variable threadCondVar;
	static mutex threadCondMutex;
	static atomic_bool exitFlag;
};

#define log0(s) log_print(get_debug_info, s)
#define log1(s,a) log_print(get_debug_info, s, a)
#define log2(s,a,b) log_print(get_debug_info, s, a, b)
#define log3(s,a,b,c) log_print(get_debug_info, s, a, b, c)
#define log4(s,a,b,c,d) log_print(get_debug_info, s, a, b, c, d)
#define log5(s,a,b,c,d,e) log_print(get_debug_info, s, a, b, c, d, e)
#define log6(s,a,b,c,d,e,f) log_print(get_debug_info, s, a, b, c, d, e, f)


template<typename... T>
void log_print(
    debug_info debug,
    string s,
    T... args
){
	boost::format fmt(s);
	string result = boost::str((fmt % ... % forward<T>(args)));

    boost::format debugPrefix("%s:%d (%s):");
    string prefix = boost::str(
        debugPrefix %
        debug.file %
        debug.line %
        debug.func
    );

	LogSystem::logOutput(prefix, "    " + result);
}

template<typename... T>
void print(
    string s,
    T... args
){
	boost::format fmt(s);
	string result = boost::str((fmt % ... % forward<T>(args)));

	LogSystem::logOutput("", result);
}

template<typename... T>
void logAndThrowError(
    string s,
    T... args
){
	boost::format fmt(s);
	string result = boost::str((fmt % ... % forward<T>(args)));

	LogSystem::logOutput("", result);
 
    throw std::runtime_error(result);
}

#endif
