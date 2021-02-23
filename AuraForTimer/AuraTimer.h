#pragma once

#ifndef AURATIMER_H
#define AURATIMER_H
#include "AURALightingSDK.h"
#include <sstream>
#include "Config.h"
#include "DateTime.h"
#include <plog/Log.h>
#include <ctime>
#include <thread>
#include <atomic>
#include <csignal>

static volatile std::atomic<bool> quit = ATOMIC_VAR_INIT(false);

static void signalHandler(int sig)
{
	if (sig == SIGINT)
	{
		PLOG_INFO << "Control-C/SIGINT detected!";
		quit.store(true);
	}
}

class AuraTimer
{
private:
	// pointers to functions in dll
	EnumerateMbControllerFunc EnumerateMbController;
	SetMbModeFunc SetMbMode;
	SetMbColorFunc SetMbColor;
	GetMbColorFunc GetMbColor;
	GetMbLedCountFunc GetMbLedCount;

	EnumerateGPUFunc EnumerateGPU;
	SetGPUModeFunc SetGPUMode;
	SetGPUColorFunc SetGPUColor;
	GetGPULedCountFunc GetGPULedCount;

	CreateClaymoreKeyboardFunc CreateClaymoreKeyboard;
	SetClaymoreKeyboardModeFunc SetClaymoreKeyboardMode;
	SetClaymoreKeyboardColorFunc SetClaymoreKeyboardColor;
	GetClaymoreKeyboardLedCountFunc GetClaymoreKeyboardLedCount;

	CreateRogMouseFunc CreateRogMouse;
	SetRogMouseModeFunc SetRogMouseMode;
	SetRogMouseColorFunc SetRogMouseColor;
	RogMouseLedCountFunc RogMouseLedCount;

	//Class variables
	int t;
	HMODULE hLib = nullptr;
	Config* conf;
	DWORD mbCount;
	DWORD ledCount;
	MbLightControl* mbLightCtrl;
	BYTE* colorArray;
	time_t morning;
	time_t day;
	time_t evening;
	time_t night;
	bool running = true;
	std::thread background;

public:
	AuraTimer(Config* c)
	{
		conf = c;
		PLOG_VERBOSE << "Object init";
		hLib = LoadLibraryA("AURA_SDK.dll");
		if (hLib == nullptr)
		{
			PLOG_ERROR << "Unable able to find AURA_SDK.dll";
			return;
		}
		setupState();
	}

	~AuraTimer()
	{
		PLOG_VERBOSE << "Object being deleted";
		conf->readConfig();
		shutdownLights();
		delete colorArray;
		delete mbLightCtrl;
	}

	void setupState()
	{
		(FARPROC&)EnumerateMbController = GetProcAddress(hLib, "EnumerateMbController");
		(FARPROC&)SetMbMode = GetProcAddress(hLib, "SetMbMode");
		(FARPROC&)SetMbColor = GetProcAddress(hLib, "SetMbColor");
		(FARPROC&)GetMbColor = GetProcAddress(hLib, "GetMbColor");
		(FARPROC&)GetMbLedCount = GetProcAddress(hLib, "GetMbLedCount");

		(FARPROC&)EnumerateGPU = GetProcAddress(hLib, "EnumerateGPU");
		(FARPROC&)SetGPUMode = GetProcAddress(hLib, "SetGPUMode");
		(FARPROC&)SetGPUColor = GetProcAddress(hLib, "SetGPUColor");
		(FARPROC&)GetGPULedCount = GetProcAddress(hLib, "GetGPULedCount");

		(FARPROC&)CreateClaymoreKeyboard = GetProcAddress(hLib, "CreateClaymoreKeyboard");
		(FARPROC&)SetClaymoreKeyboardMode = GetProcAddress(hLib, "SetClaymoreKeyboardMode");
		(FARPROC&)SetClaymoreKeyboardColor = GetProcAddress(hLib, "SetClaymoreKeyboardColor");
		(FARPROC&)GetClaymoreKeyboardLedCount = GetProcAddress(hLib, "GetClaymoreKeyboardLedCount");
		(FARPROC&)EnumerateMbController = GetProcAddress(hLib, "EnumerateMbController");

		(FARPROC&)CreateRogMouse = GetProcAddress(hLib, "CreateRogMouse");
		(FARPROC&)SetRogMouseMode = GetProcAddress(hLib, "SetRogMouseMode");
		(FARPROC&)SetRogMouseColor = GetProcAddress(hLib, "SetRogMouseColor");
		(FARPROC&)RogMouseLedCount = GetProcAddress(hLib, "RogMouseLedCount");
		mbCount = EnumerateMbController(NULL, 0);

		mbLightCtrl = new MbLightControl[mbCount];
		EnumerateMbController(mbLightCtrl, mbCount);

		// 1 means software control
		SetMbMode(mbLightCtrl[0], 1);

		// For asus Z270F, first 4 leds are back i/o, and 5th is the rgb headers
		ledCount = GetMbLedCount(mbLightCtrl[0]);
		colorArray = new BYTE[ledCount * 3];
		ZeroMemory(colorArray, ledCount * 3);
	}

	void setColors(std::string colorCsv)
	{
		int* pickedColors = convertCsv(colorCsv);
		for (size_t i = 0; i < ledCount * 3; ++i)
		{
			if (i % 3 == 0)
			{
				//red
				colorArray[i] = pickedColors[0];
			}
			if (i % 3 == 1)
			{
				//blue
				colorArray[i] = pickedColors[2];
			}
			if (i % 3 == 2)
			{
				//green
				colorArray[i] = pickedColors[1];
			}
		}
		SetMbColor(mbLightCtrl[0], colorArray, ledCount * 3);
		PLOG_DEBUG << "Motherboard has been set to " << colorCsv;
	}

	int* convertCsv(std::string colorCsv)
	{
		static int ret[3];
		std::stringstream ss(colorCsv);
		for (int i = 0; i < 3; i++)
		{
			std::string substr;
			getline(ss, substr, ',');

			ret[i] = atoi(substr.c_str());
		}
		return ret;
	}

	void mainLoop()
	{
		// first run, always start
		time_t sleepUntil = 1;

		while (!quit.load())
		{
			if (time(nullptr) >= sleepUntil)
			{
				conf->readConfig();
				loadTimeStructs();
				std::string todayWeekDay = datetime::getCurrentDay();
				time_t current = time(nullptr);

				// Default is morning, so use that
				std::string useWhichTime = "Morning";
				sleepUntil = datetime::addOneMin(day);
				time_t nextMorning = datetime::addOneMin(datetime::addOneDay(morning));
				if (difftime(current, day) >= 0 && difftime(current, evening) < 0)
				{
					useWhichTime = "Day";
					sleepUntil = datetime::addOneMin(evening);
				}
				else if (difftime(current, evening) >= 0 && difftime(current, night) < 0)
				{
					useWhichTime = "Evening";
					sleepUntil = datetime::addOneMin(night);
				}
				else if (difftime(current, night) >= 0 && difftime(current, nextMorning) < 0)
				{
					useWhichTime = "Night";
					sleepUntil = nextMorning;
				}
				char buffer[80];
				strftime(buffer, 80, "%c", localtime(&sleepUntil));

				setColors(conf->getColorCsvForDayAndTime(todayWeekDay, useWhichTime));
				PLOG_INFO << "Going to sleep until: " << buffer;
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	void startEventLoop()
	{
		signal(SIGINT, signalHandler);
		background = std::thread([this] {this->mainLoop(); });
		PLOG_INFO << "Waiting for background thread to finish";

		background.join();
	}

	void loadTimeStructs()
	{
		json times = conf->getScheduleTime();
		morning = datetime::convertTime(times["Morning"]);
		day = datetime::convertTime(times["Day"]);
		evening = datetime::convertTime(times["Evening"]);
		night = datetime::convertTime(times["Night"]);
	}

	void shutdownLights()
	{
		setColors(conf->getShutdownColor());
	}
};
#endif
