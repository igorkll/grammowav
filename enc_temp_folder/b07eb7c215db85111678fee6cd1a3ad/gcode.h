#pragma once

#define _GCODE_MUL 0.6

bool gcode_extrusion = false;
double _gcode_currentX = 0;
double _gcode_currentY = 0;
double _gcode_currentZ = 0;
double _gcode_speed = 0;

double gcode_calculateE(double dist, printer_t printer) {
    double extrudedVolume = dist * printer.layerThickness * printer.nozzleDiameter;
    double filamentVolume = (M_PI * pow(printer.filamentDiameter / 2.0, 2));
    return extrudedVolume / filamentVolume;
}

void gcode_move(FILE* outputfile, printer_t printer, double x, double y, double z) {
    x += printer.xOffset;
    y += printer.yOffset;
    z += printer.zOffset;
    if (printer.invertX) x = printer.widthX - x;
    if (printer.invertY) y = printer.depthY - y;
    if (printer.invertZ) z = printer.heightZ - z;
    if (gcode_extrusion) {
        double dist = util_dist(x, y, z, _gcode_currentX, _gcode_currentY, _gcode_currentZ);
        double extrussion = gcode_calculateE(dist, printer);
        fprintf(outputfile, "G1 X%lf Y%lf Z%lf E%lf\n", x, y, z, _GCODE_MUL * printer.extrusionMultiplier * extrussion);
    } else {
        fprintf(outputfile, "G0 X%lf Y%lf Z%lf\n", _gcode_currentX, _gcode_currentY, _gcode_currentZ + 10);
        fprintf(outputfile, "G0 X%lf Y%lf Z%lf\n", x, y, z);
    }
    _gcode_currentX = x;
    _gcode_currentY = y;
    _gcode_currentZ = z;
}

void gcode_dmove(FILE* outputfile, printer_t printer, double x, double y, double z) {
    x += printer.xOffset;
    y += printer.yOffset;
    z += printer.zOffset;
    if (printer.invertX) x = printer.widthX - x;
    if (printer.invertY) y = printer.depthY - y;
    if (printer.invertZ) z = printer.heightZ - z;
    fprintf(outputfile, "G0 X%lf Y%lf Z%lf\n", x, y, z);
    _gcode_currentX = x;
    _gcode_currentY = y;
    _gcode_currentZ = z;
}

void gcode_moveC(FILE* outputfile, printer_t printer, double x, double y, double z) {
    gcode_move(outputfile, printer, (printer.widthX / 2) + x, (printer.depthY / 2) + y, z);
}

void gcode_speed(FILE* outputfile, printer_t printer, double speed) {
    _gcode_speed = speed;
    fprintf(outputfile, "G0 F%lf\n", speed);
    fprintf(outputfile, "G1 F%lf\n", speed);
}

void gcode_fan(FILE* outputfile, printer_t printer, uint8_t value) {
    fprintf(outputfile, "M106 S%i\n", value);
}