//
//  log.hpp
//  Koumachika
//
//  Created by Toni on 6/17/19.
//
//

#ifndef log_hpp
#define log_hpp

#define debug_log(s) log("%s, %d: %s", __FUNCTION__, __LINE__, s);

template<typename... T>
void log_print(string s, T... args)
{
	boost::format fmt(s);
	string result = boost::str((fmt % ... % forward<T>(args)));

	Director::getInstance()->logOutput(result);
}

#define log log_print

#endif
