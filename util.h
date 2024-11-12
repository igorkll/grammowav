#pragma once

void util_flush(HWND hwnd) {
    InvalidateRect(hwnd, NULL, true);
    UpdateWindow(hwnd);
}

void util_filename(char* name, const char* path) {
    const char* filename = max(strrchr(path, '\\'), strrchr(path, '/'));
    strcpy(name, filename + 1);
}

void util_write(FILE* file, char* text) {
    fwrite(text, 1, strlen(text), file);
}

void util_ln(FILE* file) {
    fwrite("\n", 1, 1, file);
}

void util_writeln(FILE* file, char* text) {
    util_write(file, text);
    util_ln(file);
}

void util_writeNumber(FILE* file, int number) {
    char buffer[16];
    _itoa(number, buffer, 10);
    util_write(file, buffer);
}

void util_writeNumberln(FILE* file, int number) {
    util_writeNumber(file, number);
    util_ln(file);
}

void util_writeDouble(FILE* file, double number) {
    char buffer[16];
    sprintf(buffer, "%lf", number);
    util_write(file, buffer);
}

void util_writeDoubleln(FILE* file, double number) {
    util_writeDouble(file, number);
    util_ln(file);
}

void util_writeMove(FILE* outputfile, double x, double y, double z) {
    util_write(outputfile, "G0 X");
    util_writeDouble(outputfile, x);
    util_write(outputfile, " Y");
    util_writeDouble(outputfile, y);
    util_write(outputfile, " Z");
    util_writeDoubleln(outputfile, z);
}

int util_convertSpeed(int speed) {
    return speed * 10 * 60;
}

void util_writeSpeed(FILE* outputfile, int speed) {
    util_write(outputfile, "G0 F");
    util_writeDoubleln(outputfile, speed);
}