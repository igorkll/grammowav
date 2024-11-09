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
        .type = gui_checkbox,
        .x = 8,
        .y = 40,
        .sizeX = 16,
        .sizeY = 16,
        .text = "78 RPM",
        .onceEnable = true,
        .onceId = 1,
        .state = true,
        .data = 78
    },
    {
        .type = gui_checkbox,
        .x = 8,
        .y = 40 + 16 + 4,
        .sizeX = 16,
        .sizeY = 16,
        .text = "45 RPM",
        .onceEnable = true,
        .onceId = 1,
        .data = 45
    },
    {
        .type = gui_checkbox,
        .x = 8,
        .y = 40 + ((16 + 4) * 2),
        .sizeX = 16,
        .sizeY = 16,
        .text = "33 RPM",
        .onceEnable = true,
        .onceId = 1,
        .data = 33
    },

    {
        .type = gui_checkbox,
        .x = 8 + 100,
        .y = 40,
        .sizeX = 16,
        .sizeY = 16,
        .text = "140 microns",
        .onceEnable = true,
        .onceId = 2,
        .state = true,
        .data = 140
    },
    {
        .type = gui_checkbox,
        .x = 8 + 100,
        .y = 40 + 16 + 4,
        .sizeX = 16,
        .sizeY = 16,
        .text = "55 microns",
        .onceEnable = true,
        .onceId = 2,
        .data = 55
    }/*,

    {
        .type = gui_checkbox,
        .x = 8 + 100,
        .y = 40,
        .sizeX = 16,
        .sizeY = 16,
        .text = "",
        .flipFlop = true
    }
    */
};