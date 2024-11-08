#pragma once

char currentPath[APP_PATHLEN];
char currentName[APP_PATHLEN] = "select wav file";
bool fileSelected = false;

void load_wav(HWND hwnd) {
    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "sound file (*.wav)\0*.wav\0";
    ofn.lpstrTitle = NULL;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrFile = currentPath;
    ofn.nMaxFile = APP_PATHLEN;

    if (GetOpenFileNameA(&ofn)) {
        fileSelected = true;
        util_filename(currentName, currentPath, APP_PATHLEN);
        util_flush(hwnd);
    }
}

void save_stl(HWND hwnd) {
    if (!fileSelected) return;

    char savePath[APP_PATHLEN];

    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "sound file (*.wav)\0*.wav\0";
    ofn.lpstrTitle = NULL;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrFile = savePath;
    ofn.nMaxFile = APP_PATHLEN;

    if (GetOpenFileNameA(&ofn)) {
        int r = grammowav_wavToStl(currentPath, savePath, 78, 12, 0.1, 0.1);
        MessageBoxA(hwnd, "QWE", MB_OK, MB_ICONSTOP);
    }
}

gui_object gui_objects[] = {
    {
        .type = gui_button,
        .x = 8,
        .y = 8,
        .sizeX = 100,
        .sizeY = 25,
        .text = "load wav",
        .callback = load_wav
    },
    {
        .type = gui_button,
        .x = APP_WIDTH - 100 - 8 - 16,
        .y = APP_HEIGHT - 25 - 8 - 40,
        .sizeX = 100,
        .sizeY = 25,
        .text = "save stl",
        .callback = save_stl
    },
    {
        .type = gui_text,
        .x = 100 + 8 + 8,
        .y = 8,
        .sizeX = APP_WIDTH - 16,
        .sizeY = APP_FONTSIZE,
        .text = currentName
    }
};