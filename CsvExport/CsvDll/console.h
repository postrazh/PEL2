#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#pragma once

#include <iostream>
#include <Windows.h>

#define BANNER "///////////////////////////////////////////////\n" \
				"//                                          //\n" \
				"//            PEL CSV exporter              //\n" \
				"//                                          //\n" \
				"//////////////////////////////////////////////\n\n"

class Console 
{
	private:
		FILE* fPtr;
	public:
		Console(std::string defaultText = "", std::string windowTitle = "Console v0.1")
		{
			AllocConsole();
			freopen_s(&fPtr, "CONOUT$", "w", stdout);

			std::wstring stemp = std::wstring(windowTitle.begin(), windowTitle.end());
			LPCWSTR sw = stemp.c_str();

			SetConsoleTitle(stemp.c_str());

			if (defaultText.empty() == false)
			{
				printf_s(defaultText.c_str());
			}
		}
		~Console()
		{
			fclose(fPtr);
			FreeConsole();
		}
};

#endif
