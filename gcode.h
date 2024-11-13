#pragma once

bool _gcode_extrusion_state = false;
double _gcode_extrusion_value;

void gcode_extrusion(bool state, double value) {
    _gcode_extrusion_state = state;
    _gcode_extrusion_value = value;
}

void gcode_move(FILE* outputfile, printer_t printer, double x, double y, double z) {
    z += printer.zOffset;
    if (_gcode_extrusion_state) {
        fprintf(outputfile, "G1 X%lf Y%lf Z%lf E%lf\n", x, y, z, _gcode_extrusion_value);
    } else {
        fprintf(outputfile, "G0 X%lf Y%lf Z%lf\n", x, y, z + 10);
        fprintf(outputfile, "G0 X%lf Y%lf Z%lf\n", x, y, z);
    }
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