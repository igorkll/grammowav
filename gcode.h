#pragma once

#define _GCODE_MUL 1

bool gcode_extrusion = false;
bool gcode_needRetracted = false;
bool gcode_needDeretracted = false;
double _gcode_extrusionMultiplier = 0;
double _gcode_layerThickness = 0;
double _gcode_currentX = 0;
double _gcode_currentY = 0;
double _gcode_currentZ = 0;
double _gcode_speed = 0;

double gcode_calculateE(double dist, printer_t printer) {
    double extrudedVolume = dist * _gcode_layerThickness * printer.nozzleDiameter;
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
        double extrussionValue = _GCODE_MUL * _gcode_extrusionMultiplier * extrussion;
        //if (gcode_retracted) {
        //    extrussionValue += printer.retraction;
        //}
        if (gcode_needDeretracted) {
            fprintf(outputfile, "G1 F%lf\n", util_convertSpeed(printer, printer.retractionSpeed));
            fprintf(outputfile, "G1 X%lf Y%lf Z%lf E%lf\n",
                _gcode_currentX, _gcode_currentY, _gcode_currentZ, printer.retraction);
            fprintf(outputfile, "G1 F%lf\n", _gcode_speed);
            gcode_needDeretracted = false;
        }
        fprintf(outputfile, "G1 X%lf Y%lf Z%lf E%lf\n", x, y, z, extrussionValue);
        //fprintf(outputfile, "G1 F%lf E%lf\n", printer.retractionSpeed, -printer.retraction);
        //fprintf(outputfile, "G1 F%lf\n", _gcode_speed);
        gcode_needRetracted = true;
    } else {
        if (gcode_needRetracted && printer.retraction > 0) {
            fprintf(outputfile, "G1 F%lf\n", util_convertSpeed(printer, printer.retractionSpeed));
            fprintf(outputfile, "G1 X%lf Y%lf Z%lf E%lf\n",
                _gcode_currentX, _gcode_currentY, _gcode_currentZ + 3, -printer.retraction);
            fprintf(outputfile, "G1 F%lf\n", _gcode_speed);
            gcode_needRetracted = false;
            gcode_needDeretracted = true;
        }
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

void gcode_extrusionMultiplier(FILE* outputfile, printer_t printer, double extrusionMultiplier) {
    _gcode_extrusionMultiplier = extrusionMultiplier;
}

void gcode_layerThickness(FILE* outputfile, printer_t printer, double layerThickness) {
    _gcode_layerThickness = layerThickness;
}

void gcode_fan(FILE* outputfile, printer_t printer, uint8_t value) {
    fprintf(outputfile, "M106 S%i\n", value);
}