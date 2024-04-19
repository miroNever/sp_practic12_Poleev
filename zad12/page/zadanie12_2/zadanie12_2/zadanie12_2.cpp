#include <iostream>
#include <windows.h>
#include <vector>
#include <algorithm>

using namespace std;

const int MATRIX_SIZE = 10;
const int MAX_RANDOM_NUMBER = 999;

vector<vector<int>> matrix;
CRITICAL_SECTION cs; // Критическая секция для синхронизации доступа к общим данным
LONG twoDigitCount = 0;
int minNumber = MAX_RANDOM_NUMBER + 1;
int maxNumber = 0;

// Функция заполнения матрицы случайными числами
void fillMatrix() {
    srand(GetTickCount());
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        vector<int> row;
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            row.push_back(rand() % (MAX_RANDOM_NUMBER + 1));
        }
        matrix.push_back(row);
    }
}

// Функция для подсчета двузначных чисел, минимального и максимального чисел
DWORD WINAPI analyzeMatrix(LPVOID lpParam) {
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            int num = matrix[i][j];
            if (num >= 10 && num <= 99) {
                EnterCriticalSection(&cs);
                twoDigitCount++;
                LeaveCriticalSection(&cs);
            }
            EnterCriticalSection(&cs);
            minNumber = min(minNumber, num);
            maxNumber = max(maxNumber, num);
            LeaveCriticalSection(&cs);
        }
    }
    return 0;
}

int main() {
    InitializeCriticalSection(&cs); // Инициализируем критическую секцию

    fillMatrix(); // Заполняем матрицу случайными числами

    HANDLE threads[4];
    for (int i = 0; i < 4; ++i) {
        threads[i] = CreateThread(NULL, 0, analyzeMatrix, NULL, 0, NULL);
    }

    WaitForMultipleObjects(4, threads, TRUE, INFINITE);

    cout << "Number of two-digit numbers: " << twoDigitCount << endl;
    cout << "Min number: " << minNumber << endl;
    cout << "Max number: " << maxNumber << endl;

    DeleteCriticalSection(&cs); // Удаляем критическую секцию

    return 0;
}
