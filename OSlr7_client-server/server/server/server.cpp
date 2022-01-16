#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <Windows.h>
#include <iostream>

#define MEMORY_FILE_MAPPING "memory"
#define CS_SEM "cs_sem"
#define CL_WORK_SEM "cl_work_sem" 
#define CL_WRITE_SEM "cl_write_sem"
#define SRV_WRITE_SEM "srv_write_sem" 

using namespace std;

struct node {
    char input_string[10];
    char replace_string[3];
    int replace_index;
    int size_input_string;
};

char* replaceString(node* query_body) {
    for (int i = query_body->replace_index, j = 0; i < query_body->replace_index + 3; i++, j++) {
        query_body->input_string[i] = query_body->replace_string[j];
    }
    return query_body->input_string;
}


int main() {
    HANDLE hMemory = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(node), MEMORY_FILE_MAPPING); // создание объекта отображения в файле подкачки
    node* query = (node*)MapViewOfFile(hMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(node)); // указатель на разделяемую память

    HANDLE hCriticalSection = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CS_SEM);
    HANDLE hClientWork = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CL_WORK_SEM);
    HANDLE hClientRW = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, CL_WRITE_SEM);
    HANDLE hServer = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, SRV_WRITE_SEM);

    ReleaseSemaphore(hClientWork, 1, NULL);// ожидание создания области разделяемой памяти, после ее создания семафор устанавливается в 1
    bool server_answer = true;
    while (server_answer == true) {
        WaitForSingleObject(hServer, INFINITE);
        WaitForSingleObject(hCriticalSection, INFINITE);
        if (query->input_string == "")
            server_answer = false;
        strcpy_s(query->input_string, replaceString(query));
        ReleaseSemaphore(hCriticalSection, 1, NULL);
        ReleaseSemaphore(hClientRW, 1, NULL);
    }
    UnmapViewOfFile(query);
    CloseHandle(hMemory);
}