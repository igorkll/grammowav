#include <windows.h>
#include <wingdi.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <windowsx.h>

#define APP_NAME L"grammowav"
#define APP_WIDTH 400
#define APP_HEIGHT 400
#define APP_PATHLEN 512
#define APP_FONTSIZE 8

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

int fontHeight = 16;

typedef struct {
    uint8_t type;
    uint16_t x;
    uint16_t y;
    uint16_t sizeX;
    uint16_t sizeY;
    const char* text;
    void (*callback) (HWND hwnd);
} gui_object;

#include "gui.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;
    LOGFONT lf;
    HFONT hFont;
    HBRUSH hBrush;
    HPEN hPen;
    size_t i = 0;

    switch (msg) {
    case WM_LBUTTONDOWN: {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        for (uint8_t index = 0; index < ARRAY_SIZE(gui_objects); index++) {
            gui_object object = gui_objects[index];
            if (object.callback && xPos >= object.x && yPos >= object.y && xPos < object.x + object.sizeX && yPos < object.y + object.sizeY) {
                object.callback(hwnd);
            }
        }

        break;
    }

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        strcpy(lf.lfFaceName, "Lucida Console");
        lf.lfHeight = fontHeight;
        lf.lfItalic = 0;
        lf.lfStrikeOut = 0;
        lf.lfUnderline = 0;
        lf.lfWidth = APP_FONTSIZE;
        lf.lfWeight = APP_FONTSIZE;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfPitchAndFamily = DEFAULT_PITCH;
        lf.lfEscapement = 0;

        hFont = CreateFontIndirect(&lf);
        SelectObject(hdc, hFont);
        SetBkColor(hdc, RGB(250, 200, 100));
        SetTextColor(hdc, RGB(0, 0, 255));
        SetTextAlign(hdc, TA_CENTER | TA_BOTTOM);

        hBrush = CreateSolidBrush(RGB(250, 200, 100));
        SelectObject(hdc, hBrush);
        hPen = CreatePen(2, 2, RGB(0, 0, 255));
        SelectObject(hdc, hPen);

        for (uint8_t index = 0; index < ARRAY_SIZE(gui_objects); index++) {
            gui_object object = gui_objects[index];
            switch (object.type) {
                case 0:
                    RoundRect(hdc, object.x, object.y, object.x + object.sizeX, object.y + object.sizeY, 15, 15);
                    TextOutA(hdc, object.x + (object.sizeX / 2), object.y + (object.sizeY / 2) + (fontHeight / 2), object.text, strlen(object.text));
                    break;

                case 1:
                    TextOutA(hdc, object.x + (object.sizeX / 2), object.y + (object.sizeY / 2) + (fontHeight / 2), object.text, strlen(object.text));
                    break;
            }
        }

        ValidateRect(hwnd, NULL);
        EndPaint(hwnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void initGraphic(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // create window
    WNDCLASSW wc = {
        .style = CS_HREDRAW | CS_VREDRAW,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .lpszClassName = APP_NAME,
        .hInstance = hInstance,
        .hbrBackground = GetSysColorBrush(COLOR_3DFACE),
        .lpszMenuName = NULL,
        .lpfnWndProc = WndProc,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hIcon = LoadIcon(NULL, IDI_APPLICATION)
    };

    RegisterClassW(&wc);
    HWND hwnd = CreateWindowW(wc.lpszClassName, APP_NAME, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        (GetSystemMetrics(SM_CXSCREEN) / 2) - (APP_WIDTH / 2),
        (GetSystemMetrics(SM_CYSCREEN) / 2) - (APP_HEIGHT / 2),
        APP_WIDTH, APP_HEIGHT, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    initGraphic(hInstance, hPrevInstance, pCmdLine, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}