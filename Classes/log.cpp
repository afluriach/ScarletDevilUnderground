//
//  log.cpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#include "Prefix.h"

#include "log.hpp"

void LogSystem::debugPrefix(const string& s)
{
    if(s != prevDebugPrefix){
        logOutput(s);
        prevDebugPrefix = s;
    }
}

void LogSystem::logOutput(const string& s)
{
	bufMutex.lock();
	buf.push_back(s);
	bufMutex.unlock();

	threadCondVar.notify_one();
}

void LogSystem::initThread()
{
	std::string path = io::getWriteablePath() + std::string("log.txt");
	outputFile.open(path, std::ofstream::app | std::ofstream::ate);

	logThread = thread(&update);
}

void LogSystem::exit()
{
	exitFlag.store(true);
	logThread.join();
}

void LogSystem::update()
{
	while (!exitFlag.load())
	{
		unique_lock<mutex> mlock(threadCondMutex);
		threadCondVar.wait(
			mlock,
			[]() -> bool { return !buf.empty() || exitFlag.load(); }
		);

		if (!exitFlag.load())
		{
			std::vector<std::string> _outbuf;
			bool _print = false;

			bufMutex.lock();
			swap(_outbuf, buf);
			bufMutex.unlock();

			for (string s : _outbuf)
			{
				outputFile << s << std::endl;
				printf("%s\n", s.c_str());
			}
		}
	}
}

string LogSystem::prevDebugPrefix;

mutex LogSystem::bufMutex;
vector<string> LogSystem::buf;
ofstream LogSystem::outputFile;

thread LogSystem::logThread;
condition_variable LogSystem::threadCondVar;
mutex LogSystem::threadCondMutex;
atomic_bool LogSystem::exitFlag;
