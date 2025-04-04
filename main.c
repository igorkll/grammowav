#include <windows.h>
#include <wingdi.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <windowsx.h>

#define _USE_MATH_DEFINES
#include <math.h>

#define APP_NAME L"grammowav"
#define APP_RWIDTH 600
#define APP_RHEIGHT 400
#define APP_WIDTH (APP_RWIDTH - 15)
#define APP_HEIGHT (APP_RHEIGHT - 37)
#define APP_PATHLEN 512
#define APP_FONTSIZE 8

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

int fontHeight = 16;

typedef enum {
    gui_button,
    gui_text,
    gui_checkbox,
    gui_image,
    gui_plane,
} gui_item_type;

typedef struct gui_object gui_object;
struct gui_object {
    gui_item_type type;
    uint16_t x;
    uint16_t y;
    uint16_t sizeX;
    uint16_t sizeY;
    const char* text;
    void (*callback) (gui_object* self, HWND hwnd);

    bool state;
    bool flipFlop;
    bool offset;
    bool offsetSizeX;
    bool offsetSizeY;

    bool toTarget;
    bool onceEnable;
    int onceId;

    int data;
    int id;

    gui_object* sceneSwitch;
    size_t sceneLen;

    uint32_t color;
};

typedef struct {
    int bedTemperature;
    int diskNozzleTemperature;
    int trackNozzleTemperature;

    double widthX;
    double depthY;
    double heightZ;
    
    double xOffset;
    double yOffset;
    double zOffset;

    double nozzleDiameter;
    double filamentDiameter;
    double lineDistance;
    int circleFacesNumber;

    double diskLayerThickness;
    double trackLayerThickness;

    double diskExtrusionMultiplier;
    double trackExtrusionMultiplier;

    double retraction;
    double retractionSpeed;

    bool invertX;
    bool invertY;
    bool invertZ;

    uint8_t diskFan;
    uint8_t trackFan;

    int diskPrintSpeed;
    int trackPrintSpeed;
    int fastMoveSpeed;
} printer_t;

typedef struct {
    double rpm;

    double diskDiameter;
    double holeDiameter;
    double labelDiameter;
    double diskLayers;

    double trackWidth;
    double trackAmplitude;

    // this flag inverts the direction of the track and makes it only one, this approach allows you to make a die for casting plates. the trackWidth parameter is ignored with this flag
    bool matrix;
    bool normalizeSound;
} disk_t;


typedef struct {
    double holeDiameter;

    double membraneDiameter;
    double membraneLayers;

    double pistonDiameter;
    double pistonLayers;

    double sealSize;
    double sealLayers;
} membrane_t;

#include "util.h"
#include "gcode.h"
#include "grammowav.h"
#include "membrane.h"
#include "gui.h"

HBRUSH checkboxBrush;
HBRUSH checkedBrush;
HBRUSH uncheckedBrush;
HBRUSH objBrush;
HPEN framePen;
HFONT hFont;
HBITMAP images[1];

int random(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;
    size_t i = 0;

    switch (msg) {
    case WM_LBUTTONDOWN: {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        for (size_t index = 0; index < gui_currentLen; index++) {
            gui_object* object = &gui_current[index];
            if (xPos >= object->x && yPos >= object->y && xPos < object->x + object->sizeX && yPos < object->y + object->sizeY) {
                if (object->onceEnable) {
                    for (size_t index2 = 0; index2 < gui_currentLen; index2++) {
                        gui_object* object2 = &gui_current[index2];
                        if (object2->onceEnable && index2 != index && object2->onceId == object->onceId) {
                            object2->state = false;
                        }
                    }
                }
                if (object->flipFlop) {
                    object->state = !object->state;
                } else {
                    object->state = true;
                }
                if (object->callback) {
                    object->callback(object, hwnd);
                }
                if (object->toTarget) {
                    gui_current = gui_target;
                    gui_currentLen = gui_targetLen;
                    util_flush(hwnd);
                    break;
                } else if (object->sceneSwitch) {
                    if (random(0, 10) == 0) {
                        gui_current = gui_fun;
                        gui_currentLen = ARRAY_SIZE(gui_fun);
                    } else {
                        gui_current = object->sceneSwitch;
                        gui_currentLen = object->sceneLen;
                    }
                    gui_target = object->sceneSwitch;
                    gui_targetLen = object->sceneLen;
                    util_flush(hwnd);
                    break;
                }
                util_flush(hwnd);
            }
        }

        break;
    }

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        SelectObject(hdc, hFont);
        SetBkColor(hdc, RGB(255, 255, 255));
        SetTextColor(hdc, RGB(0, 0, 0));

        for (size_t index = 0; index < gui_currentLen; index++) {
            gui_object* object = &gui_current[index];
            switch (object->type) {
                case gui_button : {
                    SelectObject(hdc, objBrush);
                    SelectObject(hdc, framePen);
                    SetTextAlign(hdc, TA_CENTER | TA_BOTTOM);
                    RoundRect(hdc, object->x, object->y, object->x + object->sizeX, object->y + object->sizeY, 8, 8);
                    TextOutA(hdc, object->x + (object->sizeX / 2), object->y + (object->sizeY / 2) + (fontHeight / 2), object->text, strlen(object->text));
                    break;
                }

                case gui_text : {
                    SelectObject(hdc, objBrush);
                    SetTextAlign(hdc, TA_TOP | TA_LEFT);
                    TextOutA(hdc, object->x, object->y, object->text, strlen(object->text));
                    break;
                }

                case gui_checkbox : {
                    RECT rect = {
                        .top = object->y,
                        .left = object->x,
                        .right = object->x + (object->sizeX - 1),
                        .bottom = object->y + (object->sizeY - 1)
                    };
                    FillRect(hdc, &rect, checkboxBrush);

                    RECT rect2 = {
                        .top = object->y + 2,
                        .left = object->x + 2,
                        .right = object->x + (object->sizeX - 3),
                        .bottom = object->y + (object->sizeY - 3)
                    };
                    FillRect(hdc, &rect2, object->state ? checkedBrush : uncheckedBrush);
                    if (object->text) {
                        SelectObject(hdc, objBrush);
                        SetTextAlign(hdc, TA_TOP | TA_LEFT);
                        TextOutA(hdc, object->x + (object->sizeX - 1) + 8, object->y, object->text, strlen(object->text));
                    }
                    break;
                }

                case gui_image: {
                    HBITMAP hBitmap = images[object->id];
                    BITMAP bitmap;
                    GetObject(hBitmap, sizeof(bitmap), &bitmap);
                    HDC hdcMem = CreateCompatibleDC(hdc);
                    SelectObject(hdcMem, hBitmap);
                    BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
                    DeleteDC(hdcMem);
                    break;
                }

                case gui_plane: {
                    uint8_t red = (object->color >> 16) % 256;
                    uint8_t green = (object->color >> 8) % 256;
                    uint8_t blue = object->color % 256;
                    HBRUSH brush = CreateSolidBrush(RGB(red, green, blue));
                    RECT rect = {
                        .top = object->y,
                        .left = object->x,
                        .right = object->x + (object->sizeX - 1),
                        .bottom = object->y + (object->sizeY - 1)
                    };
                    FillRect(hdc, &rect, brush);
                    DeleteObject(brush);
                }
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
    LOGFONT lf;

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
    objBrush = CreateSolidBrush(RGB(255, 255, 255));
    checkboxBrush = CreateSolidBrush(RGB(0, 0, 0));
    checkedBrush = CreateSolidBrush(RGB(0, 240, 0));
    uncheckedBrush = CreateSolidBrush(RGB(0, 80, 0));
    framePen = CreatePen(0, 3, RGB(255, 0, 255));


    // create window
    WNDCLASSW wc = {
        .style = CS_HREDRAW | CS_VREDRAW,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .lpszClassName = APP_NAME,
        .hInstance = hInstance,
        .hbrBackground = CreateSolidBrush(RGB(96, 160, 200)),
        .lpszMenuName = NULL,
        .lpfnWndProc = WndProc,
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hIcon = LoadIcon(NULL, IDI_APPLICATION)
    };

    RegisterClassW(&wc);
    HWND hwnd = CreateWindowW(wc.lpszClassName, APP_NAME, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        (GetSystemMetrics(SM_CXSCREEN) / 2) - (APP_RWIDTH / 2),
        (GetSystemMetrics(SM_CYSCREEN) / 2) - (APP_RHEIGHT / 2),
        APP_RWIDTH, APP_RHEIGHT, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}

void initScene(gui_object* objs, size_t size) {
    gui_object* previous = NULL;
    for (size_t index = 0; index < size; index++) {
        gui_object* object = &objs[index];
        if (object->offset) {
            object->x += previous->x;
            object->y += previous->y;
            if (object->offsetSizeX) object->x += previous->sizeX;
            if (object->offsetSizeY) object->y += previous->sizeY;
        }
        previous = object;
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    images[0] = LoadImageA(hInstance, "grammowav.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    gui_current = gui_grammowav;
    gui_currentLen = ARRAY_SIZE(gui_grammowav);
    gui_init();
    initScene(gui_current, gui_currentLen);
    initScene(gui_membraneGenerator, ARRAY_SIZE(gui_membraneGenerator));
    initGraphic(hInstance, hPrevInstance, pCmdLine, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}