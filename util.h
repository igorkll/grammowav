#pragma once

void util_flush(HWND hwnd) {
    InvalidateRect(hwnd, NULL, true);
    UpdateWindow(hwnd);
}

void util_filename(char* name, const char* path) {
    const char* filename = max(strrchr(path, '\\'), strrchr(path, '/'));
    strcpy(name, filename + 1);
}

int util_convertSpeed(printer_t printer, int speed) {
    return speed * 10 * 60;
}

double util_dist(double x1, double y1, double z1, double x2, double y2, double z2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
}