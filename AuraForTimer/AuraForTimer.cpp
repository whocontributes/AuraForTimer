// AuraForTimer.cpp : Defines the entry point for the console application.
// run as administrator!

#define PLOG_CAPTURE_FILE
#define LOG_NAME "AuraTimer.log"
#define MAX_LOG_SIZE_BYTES 1024*1024*1024
#define MAX_LOG_FILES 7
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
#include "plog/Appenders/ConsoleAppender.h"
#include "plog/Formatters/AuraFormatter.h"
#include "AuraTimer.h"
#include "Config.h"
#include <windows.h>

int preflightCheck()
{
	int res = 0;
	if (FILE* file = fopen("AURA_SDK.dll", "r"))
	{
		fclose(file);
		res = 1;
	}
	else
	{
		MessageBox(nullptr, TEXT("AURA_SDK.dll is missing from the same folder"), TEXT("Missing Aura_SDK.dll"), MB_OK);
		res = 0;
	}
	if (FILE* file = fopen("config.json", "r"))
	{
		fclose(file);
		res = 1;
	}
	else
	{
		MessageBox(nullptr, TEXT("No configuration file(config.json) found."), TEXT("Missing config.json"), MB_OK);
		res = 0;
	}
	return res;
}

int main(int argc, char* argv[])
{
	if (preflightCheck())
	{
		static plog::RollingFileAppender<plog::AuraFormatter> fileAppender(LOG_NAME, MAX_LOG_SIZE_BYTES, MAX_LOG_FILES);
		static plog::ConsoleAppender<plog::AuraFormatter> consoleAppender;
		plog::init(plog::warning, &fileAppender).addAppender(&consoleAppender);
		Config conf;
		PLOG_DEBUG << "Main started!";
		AuraTimer aura(&conf);
		aura.startEventLoop();
	}

	return 0;
}
