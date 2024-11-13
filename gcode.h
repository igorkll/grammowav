#pragma once

void gcode_writeMove(FILE* outputfile, double x, double y, double z) {
    fprintf(outputfile, "G0 X%lf Y%lf Z%lf\n", x, y, z);
}

void gcode_writeSpeed(FILE* outputfile, int speed) {
    fprintf(outputfile, "G0 F%lf\n", speed);
}