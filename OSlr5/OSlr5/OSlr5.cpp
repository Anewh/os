#include <iostream>
#include <Windows.h>
#include <random>
#include <ctime>
#include <vector>
#include <string>

#define READERS_THREADS_COUNT 6	// A
#define WRITERS_THREADS_COUNT 4	// Б
#define SUMMARY_THREADS_COUNT WRITERS_THREADS_COUNT + READERS_THREADS_COUNT

#define GENERATED_COUNT 5 // M
#define SIZE_BUFFER 25 // N

using namespace std;

HANDLE hBufferMutex;
struct BufferWrapper {
	char* buffer;
	int i;
};

HANDLE hRcMutex;
int rc = 0;

DWORD WINAPI syncWrite(LPVOID lpParam) {
	srand(time(NULL) + (int)lpParam);
	BufferWrapper* wrapper = (BufferWrapper*)lpParam;

	for (int i = 0; i < 100; i++) {

		char newData[GENERATED_COUNT + 1] = { };
		for (int i = 0; i < GENERATED_COUNT; i++)
			newData[i] = (char)('0' + rand() % ('9' - '0'));

		WaitForSingleObject(hBufferMutex, INFINITE);

		int index = rand() % (SIZE_BUFFER - GENERATED_COUNT + 1);
		for (int i = 0; i < GENERATED_COUNT; i++)
			wrapper->buffer[i + index] = newData[i];

		cout << "Writer " << wrapper->i << " input in " << index << ": " << newData << endl;

		ReleaseMutex(hBufferMutex);
	}
	return 0;
}

DWORD WINAPI asyncRead(LPVOID lpParam) {
	BufferWrapper* wrapper = (BufferWrapper*)lpParam;

	for (int i = 0; i < 100; i++) {
		WaitForSingleObject(hRcMutex, INFINITE);
		rc++;
		if (rc == 1) WaitForSingleObject(hBufferMutex, INFINITE);
		ReleaseMutex(hRcMutex);

		cout << "Reader " << wrapper->i << " output: " << wrapper->buffer << endl;

		WaitForSingleObject(hRcMutex, INFINITE);
		rc--;
		if (rc == 0) ReleaseMutex(hBufferMutex);
		ReleaseMutex(hRcMutex);
	}
	return 0;
}

void closeMutexHandles() {
	CloseHandle(hRcMutex);
	CloseHandle(hBufferMutex);
}

int main() {
	if ((hBufferMutex = CreateMutexA(NULL, FALSE, "BufferMutex")) == NULL) {
		cout << "Mutex creating error: " << GetLastError() << endl;
		return 1;
	}

	if ((hRcMutex = CreateMutexA(NULL, FALSE, "RcMutex")) == NULL) {
		cout << "Mutex creating error: " << GetLastError() << endl;
		CloseHandle(hBufferMutex);
		return 1;
	}

	HANDLE hWriters[WRITERS_THREADS_COUNT];
	HANDLE hReaders[READERS_THREADS_COUNT];
	HANDLE hSummary[SUMMARY_THREADS_COUNT];

	BufferWrapper writerBufferWrappers[WRITERS_THREADS_COUNT];
	BufferWrapper readerBufferWrappers[READERS_THREADS_COUNT];

	char buffer[SIZE_BUFFER + 1] = { };
	for (int i = 0; i < SIZE_BUFFER; i++)
		buffer[i] = '_';

	for (int i = 0; i < WRITERS_THREADS_COUNT; i++) {
		writerBufferWrappers[i].i = i + 1;
		writerBufferWrappers[i].buffer = buffer;

		if ((hWriters[i] = CreateThread(NULL, 0, syncWrite, &writerBufferWrappers[i], NULL, 0)) == NULL) {
			cout << "Writer thread creating error: " << GetLastError() << endl;
			closeMutexHandles();
			return 1;
		}
	}

	for (int i = 0; i < READERS_THREADS_COUNT; i++) {
		readerBufferWrappers[i].i = i + 1;
		readerBufferWrappers[i].buffer = buffer;

		if ((hReaders[i] = CreateThread(NULL, 0, asyncRead, &readerBufferWrappers[i], NULL, 0)) == NULL) {
			cout << "Reader thread creating error: " << GetLastError() << endl;
			closeMutexHandles();
			return 1;
		}
	}

	int i = 0;
	for (; i < WRITERS_THREADS_COUNT; i++)
		hSummary[i] = hWriters[i];
	for (; i < SUMMARY_THREADS_COUNT; i++)
		hSummary[i] = hReaders[i - WRITERS_THREADS_COUNT];

	WaitForMultipleObjects(SUMMARY_THREADS_COUNT, hSummary, TRUE, INFINITE);
	closeMutexHandles();
	return 0;
}
