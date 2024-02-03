#include <windows.h>
#include <shellapi.h>
#include <deque>
#include "resource.h"

LPCWSTR szWindowClass = L"CURSORCIRCLE";
HWND hWnd;
NOTIFYICONDATA nid;
HMENU hPopupMenu;

const int circleSize = 200;
const int thickness = 40;
const int sleepMs = 10;
const int queSize = 500 / sleepMs;
std::deque<POINT> ptque;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
    {
        // ウィンドウ
        HRGN hRgn = CreateEllipticRgn(0, 0, circleSize, circleSize);
        HRGN hRgnInner = CreateEllipticRgn(thickness, thickness, circleSize - thickness, circleSize - thickness);
        CombineRgn(hRgn, hRgn, hRgnInner, RGN_DIFF);
        SetWindowRgn(hWnd, hRgn, TRUE); // ウィンドウを赤丸の形に切り抜く
        SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        // タスクトレイ
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hWnd;
        nid.uID = 100;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSORCIRCLE));
        lstrcpy(nid.szTip, L"Cursor◎Circle");
        Shell_NotifyIcon(NIM_ADD, &nid);
        hPopupMenu = CreatePopupMenu();
        AppendMenu(hPopupMenu, MF_STRING, 1, L"Cursor◎Circle終了");
        break;
    }
    case WM_USER + 1:
        if (lParam == WM_RBUTTONDOWN) {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd);
            TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
        }
        break;
    case WM_COMMAND:
        if (wParam == 1) {
            DestroyWindow(hWnd);
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
        RECT rect;
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
        SetBkMode(hdc, TRANSPARENT);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

ATOM RegisterMyClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CURSORCIRCLE));
    wcex.hCursor = nullptr;
    wcex.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = circleSize;
    rect.bottom = circleSize;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE); // タイトルバーとウィンドウ枠を除く中身のサイズに合わせる
    hWnd = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT, // クリック等のイベントをスルーする
        szWindowClass, nullptr,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) {
        return FALSE;
    }
    SetWindowLong(hWnd, GWL_STYLE, 0); // タイトルバーとウィンドウ枠を無くす
    ShowWindow(hWnd, SW_HIDE); // 初期状態は非表示で
    return TRUE;
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_  LPSTR lpCmdLine, _In_ int nCmdShow) {
    HANDLE hMutex = CreateMutex(NULL, FALSE, szWindowClass);
    if (hMutex == NULL) {
        return 1;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return 1;
    }
    RegisterMyClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) {
        CloseHandle(hMutex);
        return 1;
    }
    int alpha = 0, alpha_prev = 0;
    MSG msg{};
    while (IsWindow(hWnd)) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            msg.message;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Sleep(sleepMs);
        POINT pt;
        BOOL gcp_result = GetCursorPos(&pt);
        if (!gcp_result)
            continue;
        ptque.push_back(pt);
        if (ptque.size() > queSize) {
            ptque.pop_front();
        }

        POINT pt1{}, pt2{};
        int turned = 0;
        for (const auto& pt0 : ptque) {
            if (((pt0.x - pt1.x) <= -10 && 10 <= (pt1.x - pt2.x)) ||
                ((pt1.x - pt2.x) <= -10 && 10 <= (pt0.x - pt1.x)))
                turned++;
            pt2 = pt1;
            pt1 = pt0;
        }
        if (turned >= (queSize / 20)) {
            alpha = 160;
        }
        if (alpha != 0) {
            SetLayeredWindowAttributes(hWnd, 0, alpha, LWA_ALPHA); // 半透明にする
            SetWindowPos(hWnd, HWND_TOPMOST, pt.x - circleSize / 2, pt.y - circleSize / 2, circleSize, circleSize, SWP_SHOWWINDOW | SWP_NOACTIVATE);
            UpdateWindow(hWnd);
        }
        else if (alpha != alpha_prev) {
            SetWindowPos(hWnd, HWND_BOTTOM, pt.x - circleSize / 2, pt.y - circleSize / 2, circleSize, circleSize, SWP_HIDEWINDOW | SWP_NOACTIVATE);
            UpdateWindow(hWnd);
        }
        alpha_prev = alpha;
        alpha = max(alpha - 10, 0);
    }
    CloseHandle(hMutex);
    return 0;
}
