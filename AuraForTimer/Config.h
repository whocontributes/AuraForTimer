#pragma once
#include <nlohmann/json.hpp>

#ifndef CONFIG_H
#define CONFIG_H
#define DEFAULT_OFF "0,0,0"
#define DEFAULT_RED "255,0,0"

#include <plog/Log.h>
#include <fstream>
using json = nlohmann::json;

class Config
{
private:
	json config;
public:
	Config()
	{
		PLOG_VERBOSE << "Getting ready to read config.json";
		readConfig();
		updateLogLevel();
	}

	~Config()
	{
		PLOG_VERBOSE << "Object being deleted";
	}

	void readConfig()
	{
		PLOG_DEBUG << "Reloading config.json";
		std::ifstream i("config.json");
		i >> config;
	}

	void updateLogLevel()
	{
		const plog::Severity level = getLogLevel();
		plog::get()->setMaxSeverity(level);
		PLOG_DEBUG << "Logging level has been set to " << config["Configuration"]["Log_Level"];
	}

	std::string getColorCsvForDayAndTime(std::string day, std::string time)
	{
		json j = getScheduleDays();
		if (j.contains("All") && j.contains(j["All"].get<std::string>()))
		{
			day = j["All"].get<std::string>();
			PLOG_INFO << "All has been set, override other days to " << day;
		}
		PLOG_DEBUG << "Looking for color on schedule: " << day << "/" << time;

		if (j.contains(day))
		{
			if (j[day].contains(time))
			{
				return getColorCsv(j[day][time].get<std::string>(),DEFAULT_RED);
			}
			else
			{
				PLOG_WARNING << time << " does not exists for " << day << ".  Defaulting to Red";
			}
		}
		else
		{
			PLOG_WARNING << day << " does not exists in Schedule.  Defaulting to Red";
		}

		return DEFAULT_RED;
	}
	std::string getShutdownColor()
	{
		if (config.contains("Shutdown"))
		{
			return getColorCsv(config["Shutdown"].get<std::string>(), DEFAULT_OFF);
		}
		else
		{
			PLOG_DEBUG << "No shutdown color. Return " << DEFAULT_OFF;

			return DEFAULT_OFF;
		}
	}

	//Input should be a color in config.json, like RED
	std::string getColorCsv(std::string color, std::string defaultColor)
	{
		if (config["Colors"].contains(color))
		{
			PLOG_INFO << color << " has been selected";
			return config["Colors"][color];
		}
		else
		{
			PLOG_WARNING << color << " does not exists in Colors.  Defaulting to " << defaultColor;
			return defaultColor;
		}
	}

	json getScheduleTime()
	{
		return config["Schedule"]["Time"];
	}

	json getScheduleDays()
	{
		return config["Schedule"]["Day"];
	}

	plog::Severity getLogLevel()
	{
		const std::string levelFile = config["Configuration"]["Log_Level"].get<std::string>();
		plog::Severity level = plog::severityFromString(levelFile.c_str());
		if (level == plog::none)
		{
			PLOG_WARNING << levelFile << " is not a valid level. Defaulting to WARNING";
			level = plog::warning;
		}
		return level;
	}
};
#endif
