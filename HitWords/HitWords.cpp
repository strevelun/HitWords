#include "framework.h"
#include "HitWords.h"
#include <string>
#include <map>
#include <list>
#include <random>

using namespace std;

#define MAX_LOADSTRING 100
#define IDT_TIMER1 1001 
#define IDT_TIMER2 1002
#define NUM_OF_WORDS        5

int speed = 20;
int score;

HINSTANCE hInst;                              
WCHAR szTitle[MAX_LOADSTRING];            
WCHAR szWindowClass[MAX_LOADSTRING];     
HWND hWnd;

typedef struct _word
{
    int x = 0, y = 0;
} Word;

const wchar_t* words[NUM_OF_WORDS] = { L"자료구조", L"컴퓨터구조", L"운영체제", L"머신러닝", L"웹프레임워크" };

map<const wchar_t*, list<Word>> wordMap;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void UpdatePos();
void CreateWord();
void DrawWords(HDC hdc);
bool CheckWord(const wchar_t* str);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HITWORDS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HITWORDS));

    MSG msg;

    SetTimer(hWnd, IDT_TIMER1, 1000, NULL);
    SetTimer(hWnd, IDT_TIMER2, 1000, NULL);

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HITWORDS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HITWORDS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 1000, 1000, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    static int xPos;
    static wchar_t str[100];
    static int count;
    static SIZE size;

    switch (message)
    {
    case WM_CREATE:
        xPos = 0;
        count = 0;
        CreateCaret(hWnd, NULL, 5, 15);
        ShowCaret(hWnd);
        break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
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
            hdc = BeginPaint(hWnd, &ps);

            wchar_t scoreStr[100];
            swprintf_s(scoreStr, L"%d", score);

            DrawWords(hdc);
            GetTextExtentPoint(hdc, str, wcslen(str), &size);
            TextOut(hdc, xPos, 900, str, wcslen(str));
            TextOut(hdc, 100, 800, scoreStr, wcslen(scoreStr));

            MoveToEx(hdc, 0, 200, NULL);
            LineTo(hdc, 1000, 200);

            SetCaretPos(size.cx, 900);

            EndPaint(hWnd, &ps);
        }
        break;

    case WM_TIMER:
        // 화면 지우고 좌표 바꾼 후 다시 드로잉
        // tickCount = 5000 가 되면 새로운 단어 생성
        // 여기서는 좌표갱신만
        InvalidateRgn(hWnd, NULL, TRUE);

        switch (wParam)
        {
        case IDT_TIMER1: // 5초에 한번씩 생성
            CreateWord();
            break;

        case IDT_TIMER2: // 1초
            UpdatePos();
            break;
                    
        }

        break;

    case WM_CHAR:
        if (wParam == VK_BACK)
        {
            if (count > 0)
                count--;
        }
        else if (wParam == VK_RETURN) 
        {
            if (count > 0)
            {
                if (CheckWord(str)) // true 리턴하면 score++
                    score++;
                count = 0;
                str[count] = NULL;
            }
        }
        else
            str[count++] = wParam;
        str[count] = NULL;
        InvalidateRgn(hWnd, NULL, TRUE);
        break;

    case WM_DESTROY:
        KillTimer(hWnd, IDT_TIMER1);
        KillTimer(hWnd, IDT_TIMER2);
        HideCaret(hWnd);
        DestroyCaret();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void UpdatePos() 
{
    map<const wchar_t*, list<Word>>::iterator iter = wordMap.begin();
    map<const wchar_t*, list<Word>>::iterator iterEnd = wordMap.end();

    for (; iter != iterEnd; iter++)
    {
        list<Word>::iterator listIter = iter->second.begin();

        for (; listIter != iter->second.end();)
        {
            listIter->y += speed;
            if (listIter->y >= 200)
            {
                auto nextIter = ++listIter;;
                iter->second.erase(--listIter);
                listIter = nextIter;
                score--;
            }
            else
                listIter++;
        }
    }
}

void CreateWord()
{
    Word word;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> d1(0, NUM_OF_WORDS-1);
    uniform_int_distribution<int> d2(0, 900); // 화면 너비 - 단어 길이
    int idx = d1(gen);
    int xpos = d2(gen);

    word.x = xpos;

    auto found = wordMap.find(words[idx]);

    if (found == wordMap.end())
    {
        list<Word> list;
        list.push_back(word);
        wordMap.insert(make_pair(words[idx], list));
    }
    else
    {
        found->second.push_back(word);
    }
}

void DrawWords(HDC hdc)
{
    map<const wchar_t*, list<Word>>::iterator iter = wordMap.begin();
    map<const wchar_t*, list<Word>>::iterator iterEnd = wordMap.end();

    for (; iter != iterEnd; iter++)
    {
        list<Word>::iterator listIter = iter->second.begin();

        for (; listIter != iter->second.end(); listIter++)
        {
            const wchar_t* word = iter->first;
            TextOut(hdc, listIter->x, listIter->y, word, _tcslen(word));
        }
    }
}

bool CheckWord(const wchar_t* str)
{
    int idx;

    if (wcscmp(str, L"자료구조") == 0)
        idx = 0;
    else if (wcscmp(str, L"컴퓨터구조") == 0)
        idx = 1;
    else if (wcscmp(str, L"운영체제") == 0)
        idx = 2;
    else if (wcscmp(str, L"머신러닝") == 0)
        idx = 3;
    else if (wcscmp(str, L"웹프레임워크") == 0)
        idx = 4;
    else
        return false;

    auto found = wordMap.find(words[idx]);

    if (found == wordMap.end())
        return false;
    else
    {
        wordMap.erase(words[idx]);
        return true;
    }
}