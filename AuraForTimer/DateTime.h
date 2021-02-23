#pragma once
#include <sstream>
#include <ctime>
#include <iomanip>
#ifndef DATETIME_H
#define DATETIME_H

namespace datetime
{
	inline std::string getCurrentDay()
	{
		std::stringstream buffer;
		time_t t = time(nullptr);
		struct tm* tmp = localtime(&t);

		buffer << std::put_time(tmp, "%A");
		return buffer.str();
	};
	
	
	// input should be HH:MM
	inline time_t convertTime(std::string in)
	{
		time_t current = time(nullptr);
		struct tm* timeinfo = localtime(&current);
		std::istringstream is(in);
		char delimiter;
		int hour;
		int min;
		if (is >> hour >> delimiter >> min)
		{
			timeinfo->tm_hour = hour;
			timeinfo->tm_min = min;
			timeinfo->tm_isdst = -1;
			time_t tmp = mktime(timeinfo);
			return tmp;
		}
		return 0;
	};
	inline time_t addOneMin(time_t in)
	{
		return in + 60;
	}
	inline time_t addOneDay(time_t in)
	{
		struct tm* tm = localtime(&in);
		tm->tm_mday += 1;
		return mktime(tm);		
	}
};
#endif
