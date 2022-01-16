#include <iostream>
#include <Windows.h>
#include <random>
#include <ctime>
#include <vector>
#include <string>

#define READERS_COUNT 8		// A = 8
#define WRITERS_COUNT 3		// Б
#define GEN_DATA_COUNT 6	// M
#define BUFFER_SIZE 30		// N

using namespace std;

#define CS_MUTEX_NAME L"CS"
HANDLE csMutex;

#define BUFFER_MUTEX_NAME L"BUF"
HANDLE bufferMutex;

HANDLE outMutex;

struct ThreadContext {
	char* buffer;
	int threadNumber;
};

int readersCount = 0;

DWORD WINAPI threadRead(LPVOID lpParam) {
	ThreadContext* threadContext = (ThreadContext*)lpParam;

	int n = 1000;
	while (n-- > 0) {
		WaitForSingleObject(csMutex, INFINITE);
		++readersCount;
		if (readersCount == 1)
			WaitForSingleObject(bufferMutex, INFINITE);
		ReleaseMutex(csMutex);

		cout << "R" << threadContext->threadNumber << ": (" << threadContext->buffer << ")" << endl;

		WaitForSingleObject(csMutex, INFINITE);
		--readersCount;
		if (readersCount == 0)
			ReleaseMutex(bufferMutex);
		ReleaseMutex(csMutex);
	}
	return 0;
}

DWORD WINAPI threadWrite(LPVOID lpParam) {
	ThreadContext* threadContext = (ThreadContext*)lpParam;
	char* buffer = threadContext->buffer;
	srand(time(NULL) + (int)lpParam);

	int n = 1000;
	while (n-- > 0) {
		char generedData[GEN_DATA_COUNT + 1];
		char c = (char)('A' + rand() % ('Z' - 'A'));
		for (int i = 0; i < GEN_DATA_COUNT; i++)
			generedData[i] = c;
		generedData[GEN_DATA_COUNT] = '\0';

		WaitForSingleObject(bufferMutex, INFINITE);

		int position = rand() % (BUFFER_SIZE - GEN_DATA_COUNT);
		for (int i = 0; i < GEN_DATA_COUNT; i++)
			buffer[i + position] = generedData[i];

		//csMutex = OpenMutex(SYNCHRONIZE, FALSE, CS_MUTEX_NAME);
		cout << "W" << threadContext->threadNumber << ": wrote (" << generedData << ") at " << position << endl;
		//ReleaseMutex(csMutex);

		ReleaseMutex(bufferMutex);
	}
	return 0;
}

int main() {
	csMutex = CreateMutex(NULL, FALSE, CS_MUTEX_NAME);
	bufferMutex = CreateMutex(NULL, FALSE, BUFFER_MUTEX_NAME);

	char buffer[BUFFER_SIZE + 1];
	for (int i = 0; i < BUFFER_SIZE; i++)
		buffer[i] = ' ';
	buffer[BUFFER_SIZE] = '\0';

	HANDLE writerThreads[WRITERS_COUNT];
	ThreadContext writerContexts[WRITERS_COUNT];

	for (int i = 0; i < WRITERS_COUNT; i++) {
		writerContexts[i].buffer = buffer;
		writerContexts[i].threadNumber = i + 1;

		writerThreads[i] = CreateThread(0, 0, threadWrite, &writerContexts[i], 0, 0);
	}

	HANDLE readerThreads[READERS_COUNT];
	ThreadContext readerContexts[READERS_COUNT];

	for (int i = 0; i < READERS_COUNT; i++) {
		readerContexts[i].buffer = buffer;
		readerContexts[i].threadNumber = i + 1;

		readerThreads[i] = CreateThread(0, 0, threadRead, &readerContexts[i], 0, 0);
	}

	WaitForMultipleObjects(WRITERS_COUNT, writerThreads, TRUE, INFINITE);
	WaitForMultipleObjects(READERS_COUNT, readerThreads, TRUE, INFINITE);

	CloseHandle(csMutex);
	CloseHandle(bufferMutex);

	return 0;
}
