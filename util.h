#pragma once

void util_flush(HWND hwnd) {
    InvalidateRect(hwnd, NULL, true);
    UpdateWindow(hwnd);
}

void util_filename(char* name, const char* path, size_t len) {
    size_t ptr = 0;
    for (size_t i = 0; i < len; i++) {
        char chr = path[i];
        if (chr == '/' || chr == '\\') {
            ptr = 0;
        } else {
            name[ptr++] = path[i];
        }
    }
    name[ptr++] = '\0';
}