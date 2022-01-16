#include <Windows.h>
#include <string>
#include <iostream>
#include <vector>

#define CS_SEM "cs_sem"
#define CL_WORK_SEM "cl_work_sem"
#define CL_WRITE_SEM "cl_write_sem"
#define SRV_WRITE_SEM "srv_write_sem"
#define COUNTER_SEM "counter_sem"


#define COUNT_CLIENT 20
#define MAX_SIZE_QUEUE 5

using namespace std;

int wmain(int argc, wchar_t* argv[]) {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    srand(GetCurrentProcessId());

    HANDLE hCriticalSection = CreateSemaphoreA(NULL, 1, 1, CS_SEM);
    HANDLE hQueueCounter = CreateSemaphoreA(NULL, MAX_SIZE_QUEUE, MAX_SIZE_QUEUE, COUNTER_SEM);
    HANDLE hClientWork = CreateSemaphoreA(NULL, 0, 1, CL_WORK_SEM); //работа клиента с сервером
    HANDLE hClientRW = CreateSemaphoreA(NULL, 0, 1, CL_WRITE_SEM);  //записи и чтения данных клиентом
    HANDLE hServer = CreateSemaphoreA(NULL, 0, 1, SRV_WRITE_SEM);  //записи и чтения данных сервером

    STARTUPINFO si;
    PROCESS_INFORMATION piserv;
    PROCESS_INFORMATION* pi = new PROCESS_INFORMATION[COUNT_CLIENT + 1];

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&piserv, sizeof(piserv));
    for (int i = 0; i < COUNT_CLIENT; i++) {
        ZeroMemory(&pi[i], sizeof(pi[i]));
    }

    if (CreateProcess(NULL, argv[2], NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &piserv)) {
        cout << "Server created succsesful" << endl;
    }
    for (int i = 0; i != COUNT_CLIENT; i++) {
        if (CreateProcess(NULL, argv[1], NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi[i + 1]))
            cout << "Client " << i << " created" << endl;
        Sleep(100);
    }
    return 0;
}