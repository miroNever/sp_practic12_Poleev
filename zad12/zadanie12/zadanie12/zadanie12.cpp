#include "framework.h"
#include "windows.h"
#include "zadanie12.h"
#include "Tlhelp32.h"
#include "stdio.h"
#include <string>
#include <vector>
#include <psapi.h>

#include <algorithm>

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
HANDLE g_hJob1 = NULL;
HANDLE g_hJob2 = NULL;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ZADANIE12, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ZADANIE12));

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ZADANIE12));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ZADANIE12);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

void ListModules(HWND hWndProcesseList, HWND hWndModulesList, DWORD index) {
    TCHAR szBuff[1024];
    SendMessage(hWndModulesList, LB_RESETCONTENT, 0, 0);
    DWORD dwProcessId;
    SendMessage(hWndProcesseList, LB_GETTEXT, index, (LPARAM)szBuff);
    _stscanf_s(szBuff, L"PID: %d", &dwProcessId);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
    if (hProcess != NULL) {
        MODULEENTRY32 me32;
        me32.dwSize = sizeof(MODULEENTRY32);

        HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, GetProcessId(hProcess));
        if (hModuleSnap == INVALID_HANDLE_VALUE) {
            MessageBox(NULL, L"Ошибка при создании снимка модулей процесса", L"Ошибка", MB_OK | MB_ICONERROR);
            CloseHandle(hProcess);
            return;
        }

        if (!Module32First(hModuleSnap, &me32)) {
            MessageBox(NULL, L"Ошибка при чтении первого модуля", L"Ошибка", MB_OK | MB_ICONERROR);
            CloseHandle(hModuleSnap);
            return;
        }

        do {
            SendMessage(hWndModulesList, LB_ADDSTRING, 0, (LPARAM)me32.szModule);
        } while (Module32Next(hModuleSnap, &me32));

        CloseHandle(hModuleSnap);
        CloseHandle(hProcess);
    }
    else {
        MessageBox(NULL, L"Ошибка при открытии процесса", L"Ошибка", MB_OK | MB_ICONERROR);
    }
}


void ListProcesses(HWND hWndList)
{
    SendMessage(hWndList, LB_RESETCONTENT, 0, 0);
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, L"Ошибка при создании снимка процессов", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);
        return;
    }

    do
    {
        TCHAR szProcessInfo[1024];
        wsprintf(szProcessInfo, L"PID: %d, Имя процесса: %s", pe32.th32ProcessID, pe32.szExeFile);
        int nIndex = SendMessage(hWndList, LB_ADDSTRING, 0, (LPARAM)szProcessInfo);
        SendMessage(hWndList, LB_SETITEMDATA, nIndex, pe32.th32ProcessID);
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}
HANDLE CreateJob(const WCHAR* jobName) {
    static bool successMessageDisplayed = false; // Статическая переменная для отслеживания вывода сообщения

    HANDLE job = CreateJobObject(NULL, jobName);
    if (job == NULL) {
        MessageBox(NULL, L"Не удалось создать объект задания.", L"Ошибка", MB_OK | MB_ICONERROR);
        return NULL;
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo = { 0 };
    jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jobInfo, sizeof(jobInfo))) {
        MessageBox(NULL, L"Не удалось установить информацию объекта задания.", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(job);
        return NULL;
    }

    const WCHAR* exePath1 = L"C:\\Users\\andre\\OneDrive\\Рабочий стол\\page\\zadanie12_2\\x64\\Debug\\zadanie12_2.exe";
    const WCHAR* exePath2 = L"C:\\Users\\andre\\OneDrive\\Рабочий стол\\page\\zadanie12_2\\x64\\Debug\\zadanie12_3.exe";

    STARTUPINFO startupInfo1 = { sizeof(startupInfo1) };
    startupInfo1.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo1.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION processInfo1;
    if (!CreateProcess(exePath1, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo1, &processInfo1)) {
        MessageBox(NULL, L"Не удалось создать процесс для первого exe-файла.", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(job);
        return NULL;
    }
    else {
        CloseHandle(processInfo1.hThread);
    }

    if (!AssignProcessToJobObject(job, processInfo1.hProcess)) {
        MessageBox(NULL, L"Не удалось включить первый процесс в объект задания.", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(processInfo1.hProcess);
        CloseHandle(job);
        return NULL;
    }

    // Проверяем, был ли успешно запущен первый процесс
    DWORD errorCode1 = GetLastError();
    if (errorCode1 != ERROR_SUCCESS) {
        MessageBox(NULL, L"Процесс zadanie12_2.exe не был успешно запущен.", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(processInfo1.hProcess);
        CloseHandle(job);
        return NULL;
    }

    STARTUPINFO startupInfo2 = { sizeof(startupInfo2) };
    startupInfo2.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo2.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION processInfo2;
    if (!CreateProcess(exePath2, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo2, &processInfo2)) {
        MessageBox(NULL, L"Не удалось создать процесс для второго exe-файла.", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(processInfo1.hProcess);
        CloseHandle(job);
        return NULL;
    }
    else {
        CloseHandle(processInfo2.hThread);
    }

    if (!AssignProcessToJobObject(job, processInfo2.hProcess)) {
        MessageBox(NULL, L"Не удалось включить второй процесс в объект задания.", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(processInfo1.hProcess);
        CloseHandle(processInfo2.hProcess);
        CloseHandle(job);
        return NULL;
    }

    // Проверяем, был ли успешно запущен второй процесс
    DWORD errorCode2 = GetLastError();
    if (errorCode2 != ERROR_SUCCESS) {
        MessageBox(NULL, L"Процесс zadanie12_3.exe не был успешно запущен.", L"Ошибка", MB_OK | MB_ICONERROR);
        CloseHandle(processInfo1.hProcess);
        CloseHandle(processInfo2.hProcess);
        CloseHandle(job);
        return NULL;
    }

    if (!successMessageDisplayed) { // Проверяем, было ли уже выведено сообщение
        MessageBox(NULL, L"Оба процесса теперь часть объекта задания.", L"Успех", MB_OK | MB_ICONINFORMATION);
        successMessageDisplayed = true; // Устанавливаем флаг, что сообщение было выведено
    }

    return job;
}
//HANDLE CreateJob1(const WCHAR* jobName) {
//    static bool successMessageDisplayed = false; // Статическая переменная для отслеживания вывода сообщения
//
//    HANDLE job = CreateJobObject(NULL, jobName);
//    if (job == NULL) {
//        MessageBox(NULL, L"Не удалось создать объект задания.", L"Ошибка", MB_OK | MB_ICONERROR);
//        return NULL;
//    }
//
//    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo = { 0 };
//    jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
//    if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jobInfo, sizeof(jobInfo))) {
//        MessageBox(NULL, L"Не удалось установить информацию объекта задания.", L"Ошибка", MB_OK | MB_ICONERROR);
//        CloseHandle(job);
//        return NULL;
//    }
//
//    const WCHAR* exePath1 = L"C:\\Users\\andre\\OneDrive\\Рабочий стол\\page\\zadanie12_2\\x64\\Debug\\zadanie12_2.exe";
//
//    STARTUPINFO startupInfo1 = { sizeof(startupInfo1) };
//    startupInfo1.dwFlags = STARTF_USESHOWWINDOW;
//    startupInfo1.wShowWindow = SW_HIDE;
//
//    PROCESS_INFORMATION processInfo1;
//    if (!CreateProcess(exePath1, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo1, &processInfo1)) {
//        MessageBox(NULL, L"Не удалось создать процесс для первого exe-файла.", L"Ошибка", MB_OK | MB_ICONERROR);
//        CloseHandle(job);
//        return NULL;
//    }
//    else {
//        CloseHandle(processInfo1.hThread);
//    }
//
//    if (!AssignProcessToJobObject(job, processInfo1.hProcess)) {
//        MessageBox(NULL, L"Не удалось включить первый процесс в объект задания.", L"Ошибка", MB_OK | MB_ICONERROR);
//        CloseHandle(processInfo1.hProcess);
//        CloseHandle(job);
//        return NULL;
//    }
//
//    // Проверяем, был ли успешно запущен первый процесс
//    DWORD errorCode1 = GetLastError();
//    if (errorCode1 != ERROR_SUCCESS) {
//        MessageBox(NULL, L"Процесс zadanie12_2.exe не был успешно запущен.", L"Ошибка", MB_OK | MB_ICONERROR);
//        CloseHandle(processInfo1.hProcess);
//        CloseHandle(job);
//        return NULL;
//    }
//
//    if (!successMessageDisplayed) { // Проверяем, было ли уже выведено сообщение
//        MessageBox(NULL, L"Оба процесса теперь часть объекта задания.", L"Успех", MB_OK | MB_ICONINFORMATION);
//        successMessageDisplayed = true; // Устанавливаем флаг, что сообщение было выведено
//    }
//
//    return job;
//}
//
//
//HANDLE CreateJob2(const WCHAR* jobName) {
//    static bool successMessageDisplayed = false; // Статическая переменная для отслеживания вывода сообщения
//
//    HANDLE job = CreateJobObject(NULL, jobName);
//    if (job == NULL) {
//        MessageBox(NULL, L"Не удалось создать объект задания.", L"Ошибка", MB_OK | MB_ICONERROR);
//        return NULL;
//    }
//
//    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo = { 0 };
//    jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
//    if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &jobInfo, sizeof(jobInfo))) {
//        MessageBox(NULL, L"Не удалось установить информацию объекта задания.", L"Ошибка", MB_OK | MB_ICONERROR);
//        CloseHandle(job);
//        return NULL;
//    }
//
//    const WCHAR* exePath2 = L"C:\\Users\\andre\\OneDrive\\Рабочий стол\\page\\zadanie12_2\\x64\\Debug\\zadanie12_3.exe";
//
//
//
//    STARTUPINFO startupInfo2 = { sizeof(startupInfo2) };
//    startupInfo2.dwFlags = STARTF_USESHOWWINDOW;
//    startupInfo2.wShowWindow = SW_HIDE;
//
//    PROCESS_INFORMATION processInfo2;
//    if (!CreateProcess(exePath2, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo2, &processInfo2)) {
//        MessageBox(NULL, L"Не удалось создать процесс для второго exe-файла.", L"Ошибка", MB_OK | MB_ICONERROR);
//        CloseHandle(job);
//        return NULL;
//    }
//    else {
//        CloseHandle(processInfo2.hThread);
//    }
//
//    if (!AssignProcessToJobObject(job, processInfo2.hProcess)) {
//        MessageBox(NULL, L"Не удалось включить второй процесс в объект задания.", L"Ошибка", MB_OK | MB_ICONERROR);
//        CloseHandle(processInfo2.hProcess);
//        CloseHandle(job);
//        return NULL;
//    }
//
//    // Проверяем, был ли успешно запущен второй процесс
//    DWORD errorCode2 = GetLastError();
//    if (errorCode2 != ERROR_SUCCESS) {
//        MessageBox(NULL, L"Процесс zadanie12_3.exe не был успешно запущен.", L"Ошибка", MB_OK | MB_ICONERROR);
//        CloseHandle(processInfo2.hProcess);
//        CloseHandle(job);
//        return NULL;
//    }
//
//    if (!successMessageDisplayed) { // Проверяем, было ли уже выведено сообщение
//        MessageBox(NULL, L"Оба процесса теперь часть объекта задания.", L"Успех", MB_OK | MB_ICONINFORMATION);
//        successMessageDisplayed = true; // Устанавливаем флаг, что сообщение было выведено
//    }
//
//    return job;
//}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
    {
        return FALSE;
    }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    HWND hWndProcesse = GetDlgItem(hWnd, 2);
    ListProcesses(hWndProcesse);

    return TRUE;
}

HWND hComboBoxPriorityClass = NULL;
HWND hComboBoxThreadPriority = NULL;


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateWindowW(L"LISTBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_STANDARD | LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT,
            10, 90, 470, 300, hWnd, (HMENU)2, NULL, NULL);
        CreateWindowW(L"LISTBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_STANDARD | LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT,
            500, 90, 470, 300, hWnd, (HMENU)3, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Обновить список", WS_VISIBLE | WS_CHILD, 220, 50, 150, 30, hWnd, (HMENU)5, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Завершить задачу", WS_VISIBLE | WS_CHILD, 590, 50, 150, 30, hWnd, (HMENU)7, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Приостановить работу на 5сек", WS_VISIBLE | WS_CHILD, 750, 50, 220, 30, hWnd, (HMENU)8, NULL, NULL);
        CreateWindowW(L"STATIC", L"Процессы", WS_VISIBLE | WS_CHILD | SS_NOTIFY, 10, 50, 150, 20, hWnd, NULL, NULL, NULL);


        hComboBoxPriorityClass = CreateWindowW(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD,
            10, 10, 200, 200, hWnd, (HMENU)10, NULL, NULL);
        // Добавление вариантов класса приоритета в ComboBox
        SendMessage(hComboBoxPriorityClass, CB_ADDSTRING, 0, (LPARAM)L"Реального времени");
        SendMessage(hComboBoxPriorityClass, CB_ADDSTRING, 0, (LPARAM)L"Высокий");
        SendMessage(hComboBoxPriorityClass, CB_ADDSTRING, 0, (LPARAM)L"Выше среднего");
        SendMessage(hComboBoxPriorityClass, CB_ADDSTRING, 0, (LPARAM)L"Обычный");
        SendMessage(hComboBoxPriorityClass, CB_ADDSTRING, 0, (LPARAM)L"Ниже среднего");
        SendMessage(hComboBoxPriorityClass, CB_ADDSTRING, 0, (LPARAM)L"Низкий");

        hComboBoxThreadPriority = CreateWindowW(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD,
            220, 10, 200, 200, hWnd, (HMENU)11, NULL, NULL);
        // Добавление вариантов относительного приоритета в ComboBox
        SendMessage(hComboBoxThreadPriority, CB_ADDSTRING, 0, (LPARAM)L"Простаивающий");
        SendMessage(hComboBoxThreadPriority, CB_ADDSTRING, 0, (LPARAM)L"Низкий");
        SendMessage(hComboBoxThreadPriority, CB_ADDSTRING, 0, (LPARAM)L"Ниже среднего");
        SendMessage(hComboBoxThreadPriority, CB_ADDSTRING, 0, (LPARAM)L"Обычный");
        SendMessage(hComboBoxThreadPriority, CB_ADDSTRING, 0, (LPARAM)L"Выше среднег");
        SendMessage(hComboBoxThreadPriority, CB_ADDSTRING, 0, (LPARAM)L"Высокий");
        SendMessage(hComboBoxThreadPriority, CB_ADDSTRING, 0, (LPARAM)L"Реального времени");

        CreateWindowW(L"BUTTON", L"Применить", WS_VISIBLE | WS_CHILD, 430, 10, 100, 25, hWnd, (HMENU)12, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Запустить задания", WS_VISIBLE | WS_CHILD, 750, 400, 220, 30, hWnd, (HMENU)13, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Процессы в задание", WS_VISIBLE | WS_CHILD, 500, 400, 220, 30, hWnd, (HMENU)14, NULL, NULL);

        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        hComboBoxPriorityClass = GetDlgItem(hWnd, 10);
        hComboBoxThreadPriority = GetDlgItem(hWnd, 11);

        switch (wmId)
        {
        case 2:
        {
            HWND hWndProcesse = GetDlgItem(hWnd, 2);
            HWND hWndModules = GetDlgItem(hWnd, 3);
            int Index = SendMessage(hWndProcesse, LB_GETCURSEL, 0, 0);
            if (Index != LB_ERR) {
                ListModules(hWndProcesse, hWndModules, Index);
            }
            break;
        }
        case 5:
        {
            HWND hWndProcesse = GetDlgItem(hWnd, 2);
            ListProcesses(hWndProcesse);
            break;
        }

        case 7:
        {
            HWND hWndProcesse = GetDlgItem(hWnd, 2);
            int selectedIndex = SendMessage(hWndProcesse, LB_GETCURSEL, 0, 0);
            if (selectedIndex != LB_ERR) {
                DWORD dwProcessId = SendMessage(hWndProcesse, LB_GETITEMDATA, selectedIndex, 0);
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);  
                if (hProcess != NULL) {
                    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
                    if (hThreadSnap != INVALID_HANDLE_VALUE) {
                        THREADENTRY32 te32;
                        te32.dwSize = sizeof(THREADENTRY32);
                        if (Thread32First(hThreadSnap, &te32)) {
                            do {
                                if (te32.th32OwnerProcessID == dwProcessId) {
                                    HANDLE hThread = OpenThread(THREAD_TERMINATE, FALSE, te32.th32ThreadID);
                                    if (hThread != NULL) {
                                        TerminateThread(hThread, 0);
                                        CloseHandle(hThread);
                                    }
                                }
                            } while (Thread32Next(hThreadSnap, &te32));
                        }
                        CloseHandle(hThreadSnap);
                    }
                    CloseHandle(hProcess);
                }
                else {
                    MessageBox(NULL, L"Ошибка при открытии процесса для завершения.", L"Ошибка", MB_OK | MB_ICONERROR);
                }
            }
            break;
        }
        case 8: 
        {
            Sleep(5000);
            break;
        }

        case 12:
        {

            hComboBoxPriorityClass = GetDlgItem(hWnd, 10);
            hComboBoxThreadPriority = GetDlgItem(hWnd, 11);
            // Получение выбранного класса приоритета
            int selectedIndexPriorityClass = SendMessage(hComboBoxPriorityClass, CB_GETCURSEL, 0, 0);
            int priorityClass;
            switch (selectedIndexPriorityClass)
            {
                case 0: 
                    priorityClass = REALTIME_PRIORITY_CLASS; 
                    break;
                case 1: 
                    priorityClass = HIGH_PRIORITY_CLASS;
                    break;
                case 2: 
                    priorityClass = ABOVE_NORMAL_PRIORITY_CLASS; 
                    break;
                case 3: 
                    priorityClass = NORMAL_PRIORITY_CLASS; 
                    break;
                case 4: 
                    priorityClass = BELOW_NORMAL_PRIORITY_CLASS;
                    break;
                case 5: 
                    priorityClass = IDLE_PRIORITY_CLASS;
                    break;
                default:
                    priorityClass = NORMAL_PRIORITY_CLASS; 
                    break;
            }

            // Получение выбранного относительного приоритета
            int selectedIndexThreadPriority = SendMessage(hComboBoxThreadPriority, CB_GETCURSEL, 0, 0);
            int threadPriority;
            switch (selectedIndexThreadPriority)
            {
                case 0: 
                    threadPriority = THREAD_PRIORITY_IDLE; 
                    break;
                case 1: 
                    threadPriority = THREAD_PRIORITY_LOWEST; 
                    break;
                case 2: 
                    threadPriority = THREAD_PRIORITY_BELOW_NORMAL; 
                    break;
                case 3:
                    threadPriority = THREAD_PRIORITY_NORMAL;
                    break;
                case 4:
                    threadPriority = THREAD_PRIORITY_ABOVE_NORMAL; 
                    break;
                case 5:
                    threadPriority = THREAD_PRIORITY_HIGHEST; 
                    break;
                case 6: 
                    threadPriority = THREAD_PRIORITY_TIME_CRITICAL;
                    break;

                default:
                    threadPriority = THREAD_PRIORITY_NORMAL; 
                    break;
            }

            // Установка класса приоритета и относительного приоритета главного потока
            SetPriorityClass(GetCurrentProcess(), priorityClass);
            SetThreadPriority(GetCurrentThread(), threadPriority);
            break;
        }
        case 13:
        {
            // Код для запуска процессов
         // Создаем объекты задания
            g_hJob1 = CreateJob(L"JOB1");
            g_hJob2 = CreateJob(L"JOB2");

            if (g_hJob1 == NULL || g_hJob2 == NULL) {
                MessageBox(NULL, L"Не удалось создать объекты задания.", L"Ошибка", MB_OK | MB_ICONERROR);
                break;
            }

            // Выводим сообщение об успешном создании объектов задания
            MessageBox(NULL, L"Объекты задания успешно созданы.", L"Успех", MB_OK | MB_ICONINFORMATION);

            break;
        }
        case 14: {
            HWND hWndProcessesList = GetDlgItem(hWnd, 3);
            SendMessage(hWndProcessesList, LB_RESETCONTENT, 0, 0); // Очистить список процессов

            // Получить список идентификаторов процессов, включенных в задание
            DWORD processIds[1024];
            DWORD bytesReturned;
            if (!EnumProcesses(processIds, sizeof(processIds), &bytesReturned)) {
                MessageBox(NULL, L"Ошибка при перечислении процессов.", L"Ошибка", MB_OK | MB_ICONERROR);
                break;
            }

            // Получить количество процессов
            int numProcesses = bytesReturned / sizeof(DWORD);

            // Получить имя каждого процесса и добавить его в список
            for (int i = 0; i < numProcesses; ++i) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processIds[i]);
                if (hProcess != NULL) {
                    WCHAR szProcessName[MAX_PATH];
                    DWORD dwSize = MAX_PATH;
                    if (QueryFullProcessImageName(hProcess, 0, szProcessName, &dwSize)) {
                        // Извлечь только имя файла из полного пути
                        WCHAR* fileName = wcsrchr(szProcessName, L'\\');
                        if (fileName != NULL) {
                            SendMessage(hWndProcessesList, LB_ADDSTRING, 0, (LPARAM)(fileName + 1));
                        }
                    }
                    CloseHandle(hProcess);
                }
            }
            break;
        }

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }


        
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
