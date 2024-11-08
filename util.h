#pragma once

void util_flush(HWND hwnd) {
    InvalidateRect(hwnd, NULL, true);
    UpdateWindow(hwnd);
}

void util_filename(char* name, const char* path) {
    const char* filename = max(strrchr(path, '\\'), strrchr(path, '/'));
    strcpy(name, filename + 1);
}