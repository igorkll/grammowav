#pragma once

char currentPath[APP_PATHLEN];
char currentName[APP_PATHLEN] = "select wav file";
bool fileSelected = false;

void load_wav(gui_object* self, HWND hwnd) {
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
        util_filename(currentName, currentPath);
    }
}


char savePath[APP_PATHLEN];
void save_stl(gui_object* self, HWND hwnd) {
    if (!fileSelected) {
        MessageBoxA(hwnd, "first select the wav file", MB_OK, MB_ICONERROR);
        return;
    }

    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "stl record (*.stl)\0*.stl\0";
    ofn.lpstrTitle = NULL;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrFile = savePath;
    ofn.nMaxFile = APP_PATHLEN;

    if (GetSaveFileNameA(&ofn)) {
        switch (grammowav_wavToStl(currentPath, savePath, 78, 12, 0.01, 0.01)) {
            case 0:
                MessageBoxA(hwnd, "the file was saved successfully", MB_OK, MB_OK);
                break;

            case 1:
                MessageBoxA(hwnd, "the wav file could not be opened", MB_OK, MB_ICONERROR);
                break;

            case 2:
                MessageBoxA(hwnd, "the stl file could not be saved", MB_OK, MB_ICONERROR);
                break;

            case 3:
                MessageBoxA(hwnd, "the beginning of the audio data could not be found", MB_OK, MB_ICONERROR);
                break;

        }
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
        .y = 12,
        .sizeX = APP_WIDTH - 16,
        .sizeY = APP_FONTSIZE,
        .text = currentName
    },
    {
        .type = gui_image,
        .x = 4,
        .y = APP_HEIGHT - 255 - 4,
        .sizeX = 256,
        .sizeY = 256,

        .id = 0
    },

    // RPM SELECTOR
    {
        .type = gui_text,
        .x = 8,
        .y = 45,
        .sizeY = 16,
        .text = "rotation speed",
    },
    {
        .type = gui_checkbox,
        .x = 0,
        .y = 8,
        .sizeX = 16,
        .sizeY = 16,
        .text = "78 RPM",
        .data = 78,
        .state = true,

        .onceEnable = true,
        .onceId = 1,

        .offset = true,
        .offsetSizeY = true
    },
    {
        .type = gui_checkbox,
        .x = 0,
        .y = 4,
        .sizeX = 16,
        .sizeY = 16,
        .text = "45 RPM",
        .data = 45,
        
        .onceEnable = true,
        .onceId = 1,
        
        .offset = true,
        .offsetSizeY = true
    },
    {
        .type = gui_checkbox,
        .x = 0,
        .y = 4,
        .sizeX = 16,
        .sizeY = 16,
        .text = "33 RPM",
        .data = 33,

        .onceEnable = true,
        .onceId = 1,

        .offset = true,
        .offsetSizeY = true
    },
    {
        .type = gui_checkbox,
        .x = 0,
        .y = 4,
        .sizeX = 16,
        .sizeY = 16,
        .text = NULL,
        .data = 33,

        .onceEnable = true,
        .onceId = 1,

        .offset = true,
        .offsetSizeY = true
    },

    // track width selection
    {
        .type = gui_text,
        .x = 8 + 120,
        .y = 45,
        .sizeY = 16,
        .text = "track width",
    },
    {
        .type = gui_checkbox,
        .x = 0,
        .y = 8,
        .sizeX = 16,
        .sizeY = 16,
        .text = "140 microns",
        .onceEnable = true,
        .onceId = 2,
        .offset = true,
        .offsetSizeY = true,
        .state = true,
        .data = 140
    },
    {
        .type = gui_checkbox,
        .x = 0,
        .y = 4,
        .sizeX = 16,
        .sizeY = 16,
        .text = "55 microns",
        .offset = true,
        .offsetSizeY = true,
        .onceEnable = true,
        .onceId = 2,
        .data = 55
    },
    {
        .type = gui_checkbox,
        .x = 0,
        .y = 4,
        .sizeX = 16,
        .sizeY = 16,
        .text = NULL,
        .offset = true,
        .offsetSizeY = true,
        .onceEnable = true,
        .onceId = 2,
        .data = 55
    }
};