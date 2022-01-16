#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <iostream>

#define MEMORY_FILE_MAPPING "memory"
#define CS_SEM "cs_sem"
#define CL_WORK_SEM "cl_work_sem"
#define CL_WRITE_SEM "cl_write_sem"
#define SRV_WRITE_SEM "srv_write_sem"
#define COUNTER_SEM "counter_sem"

using namespace std;


struct node {
    char input_string[10];
    char replace_string[3];
    int replace_index;
    int size_input_string;
};


void createQuery(node* query_body) {
    query_body->size_input_string = 10;
    query_body->replace_index = rand() % 5;
    for (int i = 0; i < 3; i++)
        query_body->replace_string[i] = (char)(rand() % 10 + 65);
    query_body->input_string[query_body->size_input_string - 1] = '\0';
    for (int i = 0; i < query_body->size_input_string - 1; i++)
        query_body->input_string[i] = (char)(rand() % 20 + 97);
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    srand(GetCurrentProcessId());

    size_t sizeBuffer = sizeof(node);

    HANDLE hCriticalSection = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CS_SEM);
    HANDLE hQueueCounter = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, COUNTER_SEM);
    HANDLE hClientWork = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CL_WORK_SEM);
    HANDLE hClientRW = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CL_WRITE_SEM);
    HANDLE hServer = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, SRV_WRITE_SEM);

    DWORD isQueueFull = WaitForSingleObject(hQueueCounter, 1);
    if (isQueueFull == WAIT_TIMEOUT) {
        cout << "Queue owerflow: process " << GetCurrentProcessId() << " died" << endl;
        return 1;
    }

    WaitForSingleObject(hClientWork, INFINITE);
    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, MEMORY_FILE_MAPPING);
    if (hMapFile == NULL) {
        ReleaseSemaphore(hClientWork, 1, NULL);
        return 1;
    }

    node* query = (node*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeBuffer);
    if (query == NULL) {
        ReleaseSemaphore(hClientWork, 1, NULL);
        CloseHandle(hMapFile);
        return 1;
    }

    WaitForSingleObject(hCriticalSection, INFINITE);
    createQuery(query);
    cout << "Query string: " << query->input_string << endl;
    Sleep(100);
    ReleaseSemaphore(hCriticalSection, 1, NULL);

    cout << "Signal for server: query prepared" << endl;
    ReleaseSemaphore(hServer, 1, NULL);

    cout << "Wait for server..." << endl;
    WaitForSingleObject(hClientRW, INFINITE);

    cout << "**server answer: " << query->input_string << endl;
    UnmapViewOfFile(query);
    cout << "server is free" << endl;
    ReleaseSemaphore(hClientWork, 1, NULL);

    cout << "Queue size decrement" << endl << endl;
    ReleaseSemaphore(hQueueCounter, 1, NULL);
    return 0;
}
