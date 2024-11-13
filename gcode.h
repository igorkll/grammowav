#pragma once

bool gcode_extrusion = false;
double _gcode_currentX = 0;
double _gcode_currentY = 0;
double _gcode_currentZ = 0;
double _gcode_speed = 0;
bool _gcode_retractionMaked = false;

void gcode_move(FILE* outputfile, printer_t printer, double x, double y, double z) {
    z += printer.zOffset;
    if (gcode_extrusion) {
        if (_gcode_retractionMaked) {
            fprintf(outputfile, "G1 E%lf F%lf", printer.retraction, util_convertSpeed(printer, printer.retractionSpeed));
            fprintf(outputfile, "G1 F%lf\n", _gcode_speed);
        }
        double dist = util_dist(x, y, z, _gcode_currentX, _gcode_currentY, _gcode_currentZ);
        fprintf(outputfile, "G1 X%lf Y%lf Z%lf E%lf\n", x, y, z, dist * 0.2 * printer.extrusionMultiplier * (printer.nozzleDiameter / printer.filamentDiameter));
        if (printer.retraction > 0) {
            fprintf(outputfile, "G1 E-%lf F%lf", printer.retraction, util_convertSpeed(printer, printer.retractionSpeed));
            _gcode_retractionMaked = true;
        }
    } else {
        fprintf(outputfile, "G0 X%lf Y%lf Z%lf\n", x, y, z + 10);
        fprintf(outputfile, "G0 X%lf Y%lf Z%lf\n", x, y, z);
    }
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