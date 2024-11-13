#pragma once

bool gcode_extrusion = false;
double _gcode_currentX = 0;
double _gcode_currentY = 0;
double _gcode_currentZ = 0;

void gcode_move(FILE* outputfile, printer_t printer, double x, double y, double z) {
    z += printer.zOffset;
    if (gcode_extrusion) {
        double dist = util_dist(x, y, z, _gcode_currentX, _gcode_currentY, _gcode_currentZ);
        fprintf(outputfile, "G1 X%lf Y%lf Z%lf E%lf\n", x, y, z, dist * printer.extrusionMultiplier);
    } else {
        fprintf(outputfile, "G0 X%lf Y%lf Z%lf\n", x, y, z + 10);
        fprintf(outputfile, "G0 X%lf Y%lf Z%lf\n", x, y, z);
    }
    _gcode_currentX = x;
    _gcode_currentY = y;
    _gcode_currentZ = z;
}

void gcode_moveC(FILE* outputfile, printer_t printer, double x, double y, double z) {
    gcode_move(outputfile, printer, (printer.widthX / 2) + x, (printer.depthY / 2) + y, 0);
}

void gcode_speed(FILE* outputfile, printer_t printer, double speed) {
    fprintf(outputfile, "G0 F%lf\n", speed);
}

void gcode_fan(FILE* outputfile, printer_t printer, uint8_t value) {
    fprintf(outputfile, "M106 S%i\n", value);
}