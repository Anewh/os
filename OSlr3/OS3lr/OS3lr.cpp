#include <iostream>
#include <Windows.h>
#include <ctime>

#define MULTIPLICITY 3
#define MATRIX_SIZE 5

using namespace std;

struct MatrixRow {
	int row[MATRIX_SIZE];
	int result;
};

void findFirstMultipleElement(int* row, int multiplicity, int& result) {
	for (int i = 0; i < MATRIX_SIZE; i++) {
		if (row[i] % 3 == 0) {
			result = row[i];
			break;
		}
	}
}

DWORD WINAPI createRow(LPVOID lpParam) {
	srand(time(NULL) + (int)lpParam);
	MatrixRow* matrixRow = (MatrixRow*)lpParam;

	for (int i = 0; i < MATRIX_SIZE; i++)
		matrixRow->row[i] = rand() % 100;

	matrixRow->result = 0; 
	findFirstMultipleElement(matrixRow->row, MULTIPLICITY, matrixRow->result);

	return 0;
}

DWORD WINAPI printMatrix(LPVOID lpParam) {
	MatrixRow* matrixRows = (MatrixRow*)lpParam;

	for (int i = 0; i < MATRIX_SIZE; i++) {
		int* row = matrixRows[i].row;

		(( matrixRows[i].result != 0 ) ? cout << "Multiple " << MULTIPLICITY <<  " element : " << matrixRows[i].result 
			: cout << "Multiple " << MULTIPLICITY << " element not found :( ");

		cout << "\nRow: ";
		for (int j = 0; j < MATRIX_SIZE; j++)
			cout << row[j] << " ";
		cout << endl << endl;
	}
	return 0;
}


int main() {
	srand(time(NULL));
	MatrixRow matrix[MATRIX_SIZE];
	HANDLE threadGenerate[MATRIX_SIZE];

	for (int i = 0; i < MATRIX_SIZE; i++) {
		threadGenerate[i] = CreateThread(NULL, 0, createRow, &matrix[i], 0, NULL);
	}
	WaitForMultipleObjects(MATRIX_SIZE, threadGenerate, TRUE, INFINITE);

	HANDLE threadOutput = CreateThread(NULL, 0, printMatrix, matrix, 0, NULL);
	WaitForSingleObject(threadOutput, INFINITE);

	return 0;
}

