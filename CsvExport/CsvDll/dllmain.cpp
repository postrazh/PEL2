// dllmain.cpp : Defines the entry point for the DLL application.

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdlib.h>
#include <time.h>

#include <tchar.h>
#include "framework.h"
#include "console.h"
#include "mem.h"
#include "CSVWriter.h"

#define IPC_IMPLEMENTATION
#include "ipc.h"

//Console instance
Console* console;

DWORD WINAPI dwMain(LPVOID lpArg);

uintptr_t moduleBase = NULL;

// variables
ipc_sharedmemory shared_mem;

int g_interval_seconds = 0;
int g_reset_table_minutes = 0;
int g_flag = 0;
std::string g_csv_path;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	DWORD dwThreadID = 0;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		//Initialize instance of the the console
		//console = new Console(std::string(BANNER), std::string("PEL console\n"));

		//Start main thread
		CreateThread(NULL, NULL, &dwMain, NULL, NULL, &dwThreadID);

		break;

    case DLL_THREAD_ATTACH:
		break;

    case DLL_THREAD_DETACH:
		break;

    case DLL_PROCESS_DETACH:
		//Free console
		printf_s("DLL_PROCESS_DETACH\n");
		delete console;
        break;
    }
    return TRUE;
}

uintptr_t getObjectPtr()
{
	uintptr_t obj_addr = mem::FindDMAAddy(moduleBase, moduleBase + 0x001B353C, { 0x30, 0x20, 0x1FC });
	return obj_addr;
}

double getDoubleValue(int a1, int a2)
{
	uintptr_t obj_addr = getObjectPtr();

	DWORD _RMSGetValue = moduleBase + 0x6FC20;

	double value = 0;

	__asm
	{
		mov ecx, obj_addr
		push ecx
		mov eax, a2
		push eax
		mov eax, a1
		push eax
		call _RMSGetValue
		movsd value, xmm0
	}

	return value;
}


void printRMS()
{
	//////////////////////////////////////////////////////////////////
	printf_s("RMS\n");
	printf_s("V1 = %0.1f V\n", getDoubleValue(0, 0));
	printf_s("V2 = %0.1f V\n", getDoubleValue(0, 1));
	printf_s("V3 = %0.1f V\n", getDoubleValue(0, 2));

	printf_s("U12 = %0.1f V\n", getDoubleValue(2, 0));
	printf_s("U23 = %0.1f V\n", getDoubleValue(2, 1));
	printf_s("U31 = %0.1f V\n", getDoubleValue(2, 2));

	printf_s("I1 = %0.1f mA\n", getDoubleValue(3, 0) * 1000.0f);
	printf_s("I2 = %0.1f mA\n", getDoubleValue(3, 1) * 1000.0f);
	printf_s("I3 = %0.1f mA\n", getDoubleValue(3, 2) * 1000.0f);
	printf_s("IN = %0.1f mA\n", getDoubleValue(3, 3) * 1000.0f);

	//////////////////////////////////////////////////////////////////
	printf_s("\nTHD \n");
	printf_s("V1-THD = %0.2f %%\n", getDoubleValue(38, 0));
	printf_s("V2-THD = %0.2f %%\n", getDoubleValue(38, 1));
	printf_s("V3-THD = %0.2f %%\n", getDoubleValue(38, 2));

	printf_s("U12-THD = %0.2f %%\n", getDoubleValue(39, 0));
	printf_s("U23-THD = %0.2f %%\n", getDoubleValue(39, 1));
	printf_s("U31-THD = %0.2f %%\n", getDoubleValue(39, 2));

	printf_s("I1-THD = %0.2f %%\n", getDoubleValue(40, 0));
	printf_s("I2-THD = %0.2f %%\n", getDoubleValue(40, 1));
	printf_s("I3-THD = %0.2f %%\n", getDoubleValue(40, 2));
	printf_s("IN-THD = %0.2f %%\n", getDoubleValue(40, 3));


	//////////////////////////////////////////////////////////////////
	printf_s("\nCrest Factor \n");
	printf_s("V1-CF = %0.3f\n", getDoubleValue(5, 0));
	printf_s("V2-CF = %0.3f\n", getDoubleValue(5, 1));
	printf_s("V3-CF = %0.3f\n", getDoubleValue(5, 2));

	printf_s("I1-CF = %0.3f\n", getDoubleValue(6, 0));
	printf_s("I2-CF = %0.3f\n", getDoubleValue(6, 1));
	printf_s("I3-CF = %0.3f\n", getDoubleValue(6, 2));

	//////////////////////////////////////////////////////////////////
	printf_s("\nF (Hz) = %0.2f Hz \n", getDoubleValue(8, 4));
	printf_s("Vunb (u2) = %0.2f Hz \n", getDoubleValue(7, 4));

}



double getIntValue(int a1, int a2)
{
	uintptr_t obj_addr = getObjectPtr();

	DWORD _PQSGetValue = moduleBase + 0x70CD0;

	int value = 0;
	double dfValue = 0;

	__asm
	{
		mov ecx, obj_addr
		mov eax, a2
		push eax
		mov eax, a1
		push eax
		call _PQSGetValue
		mov value, eax
	}

	dfValue = value / 1000.0f;

	return dfValue;
}

std::string getDoubleString1fMult1000(int a1, int a2)
{
	std::stringstream stream;
	stream.precision(1);
	stream << std::fixed << (getDoubleValue(a1, a2) * 1000.0f);

	return stream.str();
}


std::string getDoubleString1f(int a1, int a2)
{
	std::stringstream stream;
	stream.precision(1);
	stream << std::fixed << getDoubleValue(a1, a2);
	
	return stream.str();
}

std::string getDoubleString2f(int a1, int a2)
{
	std::stringstream stream;
	stream.precision(2);
	stream << std::fixed << getDoubleValue(a1, a2);

	return stream.str();
}


std::string getDoubleString3f(int a1, int a2)
{
	std::stringstream stream;
	stream.precision(3);
	stream << std::fixed << getDoubleValue(a1, a2);

	return stream.str();
}


std::string getIntString3f(int a1, int a2)
{
	std::stringstream stream;
	stream.precision(3);
	stream << std::fixed << getIntValue(a1, a2);

	return stream.str();
}

void printPQS()
{
	//////////////////////////////////////////////////////////////////
	printf_s("PQS\n");
	printf_s("P1 = %0.3f kW\n", getIntValue(9, 0));
	printf_s("P2 = %0.3f kW\n", getIntValue(9, 1));
	printf_s("P3 = %0.3f kW\n", getIntValue(9, 2));
	printf_s("PT = %0.3f kW\n", getIntValue(9, 5));

	printf_s("Q1 = %0.3f kvar\n", getIntValue(12, 0));
	printf_s("Q2 = %0.3f kvar\n", getIntValue(12, 1));
	printf_s("Q3 = %0.3f kvar\n", getIntValue(12, 2));
	printf_s("QT = %0.3f kvar\n", getIntValue(12, 5));

	printf_s("S1 = %0.3f kVA\n", getIntValue(15, 0));
	printf_s("S2 = %0.3f kVA\n", getIntValue(15, 1));
	printf_s("S3 = %0.3f kVA\n", getIntValue(15, 2));
	printf_s("ST = %0.3f kVA\n", getIntValue(15, 5));

	//////////////////////////////////////////////////////////////////
	printf_s("\nCos Phi\n");
	printf_s("Cos Phi1 = %0.3f \n", getDoubleValue(20, 0));
	printf_s("Cos Phi2 = %0.3f \n", getDoubleValue(20, 1));
	printf_s("Cos Phi3 = %0.3f \n", getDoubleValue(20, 2));
	printf_s("Cos PhiT = %0.3f \n", getDoubleValue(20, 4));

	//////////////////////////////////////////////////////////////////
	printf_s("\nPower Factor\n");
	printf_s("PF1 = %0.3f \n", getDoubleValue(16, 0));
	printf_s("PF2 = %0.3f \n", getDoubleValue(16, 1));
	printf_s("PF3 = %0.3f \n", getDoubleValue(16, 2));
	printf_s("PFT = %0.3f \n", getDoubleValue(16, 5));

	//////////////////////////////////////////////////////////////////
	printf_s("\nTan Phi\n");
	printf_s("Tan Phi = %0.3f \n", getDoubleValue(26, 4));
}

bool isConnected()
{
	uintptr_t obj_addr = getObjectPtr();

	if (obj_addr < moduleBase)
		return false;

	int* pConnected = (int*)(obj_addr + 0x28);
	int connected = *pConnected;
	
	return !connected;
}

void getConfig()
{
	// create IPC mem
	ipc_mem_init(&shared_mem, (char*)"ipc_PEL_memory", 1024);

	if (ipc_mem_open_existing(&shared_mem) == 0)
	{
		int *p = (int*)shared_mem.data;
		// read interval_seconds
		g_interval_seconds = *p;
		p++;

		// read reset_table_minutes
		g_reset_table_minutes = *p;
		p++;

		// read flag
		g_flag = *p;
		p++;

		// read csv_path
		TCHAR buf[MAX_PATH];
		lstrcpyn(buf, (LPCWSTR)p, MAX_PATH);

		std::wstring csv_path_w = buf;
		g_csv_path = std::string(csv_path_w.begin(), csv_path_w.end());
	}

	// clean up
	ipc_mem_close(&shared_mem);
}

inline bool is_exists(const std::string& name) {
	if (FILE *file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

void delete_file(const std::string& name)
{
	remove(name.c_str());
}

std::string getTimeStamp()
{
	time_t	now = time(0);
	struct tm	tstruct;
	char		buf[MAX_PATH];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);

	std::string timestamp(buf);

	return timestamp;
}

void WriteCsvHeader()
{
	CSVWriter csv(g_csv_path);
			
	csv.newRow() << "Timestamp"
		// RMS
		<< "V1" << "V2" << "V3"
		<< "U12" << "U23" << "U31"
		<< "I1" << "I2" << "I3" << "IN"
		<< "V1-THD" << "V2-THD" << "V3-THD"
		<< "U12-THD" << "U23-THD" << "U31-THD"
		<< "I1-THD" << "I2-THD" << "I3-THD" << "IN-THD"
		<< "V1-CF" << "V2-CF" << "V3-CF"
		<< "I1-CF" << "I2-CF" << "I3-CF"
		<< "F (Hz)" << "Vunb (u2)"

		// PQS
		<< "P1" << "P2" << "P3" << "PT"
		<< "Q1" << "Q2" << "Q3" << "QT"
		<< "S1" << "S2" << "S3" << "ST"
		<< "Cos Phi1" << "Cos Phi2" << "Cos Phi3" << "Cos PhiT"
		<< "PF1" << "PF2" << "PF3" << "PFT"
		<< "Tan Phi";

	if (!csv.writeToFile(true)) {
		printf_s("Can not write header to the csv file.\n");
	}
}

void WriteCsvRow()
{
	CSVWriter csv(g_csv_path);
	//CSVWriter csv("ab");

	csv.newRow();

	csv << getTimeStamp()
		/////////////// RMS ///////////////////////
		// V1, V2, V3
		<< getDoubleString1f(0, 0)
		<< getDoubleString1f(0, 1)
		<< getDoubleString1f(0, 2)

		// U12, U23, U31
		<< getDoubleString1f(2, 0)
		<< getDoubleString1f(2, 1)
		<< getDoubleString1f(2, 2)

		// I1, I2, I3, IN
		<< getDoubleString1fMult1000(3, 0)
		<< getDoubleString1fMult1000(3, 1)
		<< getDoubleString1fMult1000(3, 2)
		<< getDoubleString1fMult1000(3, 3)

		// V1-THD, V2-THD, V3-THD
		<< getDoubleString2f(38, 0)
		<< getDoubleString2f(38, 1)
		<< getDoubleString2f(38, 2)

		// U12-THD, U23-THD, U31-THD
		<< getDoubleString2f(39, 0)
		<< getDoubleString2f(39, 1)
		<< getDoubleString2f(39, 2)

		// I1-THD, I2-THD, I3-THD, IN-THD
		<< getDoubleString2f(40, 0)
		<< getDoubleString2f(40, 1)
		<< getDoubleString2f(40, 2)
		<< getDoubleString2f(40, 3)

		// V1-CF, V2-CF, V3-CF
		<< getDoubleString3f(5, 0)
		<< getDoubleString3f(5, 1)
		<< getDoubleString3f(5, 2)

		// I1-CF, I2-CF, I3-CF
		<< getDoubleString3f(6, 0)
		<< getDoubleString3f(6, 1)
		<< getDoubleString3f(6, 2)

		// F (Hz), Vunb (u2)
		<< getDoubleString2f(8, 4)
		<< getDoubleString2f(7, 4)

		/////////////// PQS ///////////////////////

		// P1, P2, P3, PT
		<< getIntString3f(9, 0)
		<< getIntString3f(9, 1)
		<< getIntString3f(9, 2)
		<< getIntString3f(9, 5)

		// Q1, Q2, Q3, QT
		<< getIntString3f(12, 0)
		<< getIntString3f(12, 1)
		<< getIntString3f(12, 2)
		<< getIntString3f(12, 5)

		// S1, S2, S3, ST
		<< getIntString3f(15, 0)
		<< getIntString3f(15, 1)
		<< getIntString3f(15, 2)
		<< getIntString3f(15, 5)

		// Cos Phi1, Cos Phi2, Cos Phi3, Cos PhiT
		<< getDoubleString3f(20, 0)
		<< getDoubleString3f(20, 1)
		<< getDoubleString3f(20, 2)
		<< getDoubleString3f(20, 4)

		// PF1, PF2, PF3, PFT
		<< getDoubleString3f(16, 0)
		<< getDoubleString3f(16, 1)
		<< getDoubleString3f(16, 2)
		<< getDoubleString3f(16, 5)

		// Tan Phi
		<< getDoubleString3f(26, 4);
	
	
	if (!csv.writeToFile(true)) {
		printf_s("Can not write a row to the csv file.\n");
	}
}

/*
Main Thread

@return [h4x success]
*/
DWORD WINAPI dwMain(LPVOID lpArg)
{
	int reset_limit = 0;
	int reset_counter = 0;

	// initial sleep
	Sleep(5000);

	//
	//int *p = 0;
	//*p = 0;

	// get the config data
	getConfig();

	if (g_interval_seconds < 1)
		g_interval_seconds = 1;

	if (g_reset_table_minutes < 1)
		g_reset_table_minutes = 1;

	reset_limit = g_reset_table_minutes * 60;
	reset_counter = reset_limit;

	// capture the values
	moduleBase = (uintptr_t)GetModuleHandle(NULL);
	
	printf_s("Press [NUMPAD 1] to see RMS\n");
	printf_s("Press [NUMPAD 2] to see PQS\n");

	int rnd_ratio = rand() % 7 + 4;

	while (1)
	{
		// sleep
		Sleep(g_interval_seconds * 1000);

		// check if reset the csv table
		reset_counter -= g_interval_seconds;
		if (reset_counter <= 0)
		{
			delete_file(g_csv_path);
			// write the csv header if needed
			if (!is_exists(g_csv_path)) {
				WriteCsvHeader();
			}

			reset_counter = reset_limit;
		}

		// check the flag
		if (g_flag)
		{
			int rnd_occur = rand() % rnd_ratio;
			if (rnd_occur == 0)
				continue;
		}

		// write to csv
		bool bConnected = isConnected();

		if (bConnected) {
			WriteCsvRow();
		} 
		

		//// keyboard
		//if (GetAsyncKeyState(VK_NUMPAD1) & 1)
		//{
		//	csv.newRow() << "this" << "is" << "the" << "first" << "row";
		//	csv.newRow() << "this" << "is" << "the" << "second" << "row";
		//	if (!csv.writeToFile("foobar.csv")) {
		//		printf_s("Can not write to the csv file.\n");
		//	}


		//	if (!isConnected()) {
		//		printf_s("Not connected.\n");
		//		continue;
		//	}

		//	printf_s("----------------- RMS --------------- \n");

		//	printRMS();

		//	printf_s("------------------------------------- \n\n");
		//}

		//if (GetAsyncKeyState(VK_NUMPAD2) & 1)
		//{
		//	if (!isConnected()) {
		//		printf_s("Not connected.\n");
		//		continue;
		//	}

		//	printf_s("----------------- PQS --------------- \n");

		//	printPQS();

		//	printf_s("------------------------------------- \n\n");
		//}

	}

	return TRUE;
}

