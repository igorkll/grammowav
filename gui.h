#pragma once

char currentPath[APP_PATHLEN] = "select wav file";
bool fileSelected = false;

void load_wav(HWND hwnd) {
    char pathbuff[APP_PATHLEN];

    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "*.wav";
    ofn.lpstrTitle = "Select Wav File";
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrFile = pathbuff;
    ofn.nMaxFile = APP_PATHLEN;

    if (GetOpenFileNameA(&ofn)) {
        
    }
}

gui_object gui_objects[] = {
    {
        .type = 1,
        .x = 8,
        .y = 8,
        .sizeX = APP_WIDTH - 16,
        .sizeY = APP_FONTSIZE,
        .text = currentPath
    },
    {
        .type = 0,
        .x = 8,
        .y = 30,
        .sizeX = 100,
        .sizeY = 25,
        .text = "load wav",
        .callback = load_wav
    }
    
};