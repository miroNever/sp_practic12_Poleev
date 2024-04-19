#include <iostream>
#include <Windows.h>

using namespace std;

const int MAX_COUNT = 3;

HANDLE mutex; // Объявление мьютекса для синхронизации доступа к общему ресурсу

DWORD WINAPI Count(LPVOID lpParam) {
    int id = reinterpret_cast<int>(lpParam);

    for (int i = 0; i < MAX_COUNT; ++i) {
        WaitForSingleObject(mutex, INFINITE); // Захватываем мьютекс перед доступом к общему ресурсу

        for (int number = 0; number <= 10; ++number) {
            std::cout << id + 1 << " поток - " << number << endl;
            Sleep(150); // Имитация выполнения работы
        }

        ReleaseMutex(mutex); // Освобождаем мьютекс после завершения доступа к общему ресурсу
    }

    return 0;
}

int main() {
    setlocale(LC_ALL, "ru");

    mutex = CreateMutexA(NULL, FALSE, NULL); // Создание мьютекса
    if (mutex != NULL) {
        const int CountThreads = 3;

        HANDLE threads[CountThreads];

        for (int i = 0; i < CountThreads; ++i) {
            threads[i] = CreateThread(NULL, 0, Count, reinterpret_cast<LPVOID>(i), 0, NULL); // Создание потоков

            if (threads[i] == NULL) {
                cout << "Ошибка в создании потока" << i << endl;
                return 1;
            }
        }

        WaitForMultipleObjects(CountThreads, threads, TRUE, INFINITE); // Ожидание завершения всех потоков

        CloseHandle(mutex); // Закрытие мьютекса
    }

    return 0;
}
