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
void save_gcode(gui_object* self, HWND hwnd) {
    if (!fileSelected) {
        MessageBoxA(hwnd, "first select the wav file", MB_OK, MB_ICONERROR);
        return;
    }

    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "gcode record (*.gcode)\0*.gcode\0";
    ofn.lpstrTitle = NULL;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrFile = savePath;
    ofn.nMaxFile = APP_PATHLEN;

    if (GetSaveFileNameA(&ofn)) {
        printer_t printer = {
            .bedTemperature = 65,
            .diskNozzleTemperature = 215,
            .trackNozzleTemperature = 210,

            .widthX = 220,
            .depthY = 220,
            .heightZ = 250,
            .zOffset = 0.3 * 0.9,

            .nozzleDiameter = 0.4,
            .filamentDiameter = 1.75,
            .lineDistance = 0.4 * 0.9,
            .circleFacesNumber = 32,
            .layerThickness = 0.3,

            .diskExtrusionMultiplier = 1,
            .trackExtrusionMultiplier = 1,

            .retraction = 2,
            .retractionSpeed = 5,

            .diskFan = 60,
            .trackFan = 255,

            .diskPrintSpeed = 20,
            .trackPrintSpeed = 5,
            .fastMoveSpeed = 300
        };

        disk_t disk = {
            .rpm = 78,

            .diskDiameter = 50,
            .holeDiameter = 8,
            .labelDiameter = 10,
            .diskHeight = 0.6,

            .trackWidth = 0.5,
            .trackHeight = 0.3,
            .trackAmplitude = 0.05,

            .matrix = false,
            .normalizeSound = true
        };

        switch (grammowav_wavToGcode(currentPath, savePath, printer, disk)) {
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

            case 4:
                MessageBoxA(hwnd, "insufficient RAM", MB_OK, MB_ICONERROR);
                break;

            case 5:
                MessageBoxA(hwnd, "there is not enough space on disk for the specified audio", MB_OK, MB_ICONERROR);
                break;

            default:
                MessageBoxA(hwnd, "unknown error", MB_OK, MB_ICONERROR);
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
        .text = "save gcode",
        .callback = save_gcode
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