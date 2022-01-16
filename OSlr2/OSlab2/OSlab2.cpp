#define _CRT_SECURE_NO_WARNINGS

#include <iostream>

#include <Windows.h>

#include <vector>

#include<locale.h>

using namespace std;

//Число процессов - 3

//Порядок создания процессов - вначале параллельно первый и второй, после того, как оба завершатся, – третий

//способ перенаправления - изменения в STARTUPINFO

//порядок перенаправления - ввод/вывод, ввод, вывод

STARTUPINFO GetSI(HANDLE inputHndl, HANDLE OutputHndl) {

	STARTUPINFO si;

	GetStartupInfo(&si);

	si.dwFlags = STARTF_USESTDHANDLES;

	if (inputHndl == NULL)

		si.hStdInput = inputHndl;

	else

		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

	if (inputHndl == NULL)

		si.hStdOutput = OutputHndl;

	else

		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	return si;

}

int main() {

	setlocale(LC_ALL, "rus");

	STARTUPINFO si;

	vector<PROCESS_INFORMATION> PI(3);

	TCHAR App1Path[] = TEXT("sort.exe");

	TCHAR App2Path[] = TEXT("systeminfo.exe");

	TCHAR App3Path[] = TEXT("Counter.exe");

	TCHAR InputApp1[] = TEXT("InputApp1.txt");

	TCHAR InputApp2[] = TEXT("InputApp2.txt");

	TCHAR OutputApp1[] = TEXT("OutputApp1.txt");

	TCHAR OutputApp3[] = TEXT("OutputApp3.txt");

	SECURITY_ATTRIBUTES SA = { sizeof(SECURITY_ATTRIBUTES) , NULL, true };

	HANDLE InputApp1Hndl = CreateFile(InputApp1, GENERIC_READ, FILE_SHARE_READ, &SA, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	HANDLE InputApp2Hndl = CreateFile(InputApp2, GENERIC_READ, FILE_SHARE_READ, &SA, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	HANDLE OutputApp1Hndl = CreateFile(OutputApp1, GENERIC_WRITE, FILE_SHARE_READ, &SA, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	HANDLE OutputApp3Hndl = CreateFile(OutputApp3, GENERIC_WRITE, FILE_SHARE_READ, &SA, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	HANDLE ArrayOfEventHandles[2];

	CreateProcess(NULL, App1Path, NULL, NULL, TRUE, 0, NULL, NULL, &GetSI(InputApp1Hndl, OutputApp1Hndl), &PI[0]);

	CreateProcess(NULL, App1Path, NULL, NULL, TRUE, 0, NULL, NULL, &GetSI(InputApp1Hndl, NULL), &PI[1]);

	ArrayOfEventHandles[0] = PI.at(0).hProcess;

	ArrayOfEventHandles[1] = PI.at(1).hProcess;

	WaitForMultipleObjects(2, ArrayOfEventHandles, TRUE, INFINITE);

	CreateProcess(NULL, App2Path, NULL, NULL, TRUE, 0, NULL, NULL, &GetSI(NULL, OutputApp3), &PI[4]);

	HANDLE CalcHndl = PI.at(2).hProcess;

	WaitForSingleObject(CalcHndl, INFINITE);

	CloseHandle(PI[0].hProcess);

	CloseHandle(PI[1].hProcess);

	CloseHandle(PI[2].hProcess);

	CloseHandle(InputApp1Hndl);

	CloseHandle(InputApp2Hndl);

	CloseHandle(OutputApp1Hndl);

	CloseHandle(OutputApp3Hndl);

	SetProcessWorkingSetSize(ArrayOfEventHandles[0], -1, -1);

	SetProcessWorkingSetSize(ArrayOfEventHandles[1], -1, -1);

	return 0;

}

