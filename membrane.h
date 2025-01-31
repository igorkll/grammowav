#pragma once

int membrane_generate(const char* exportPath, printer_t printer, membrane_t membrane) {
	FILE* outputfile = fopen(exportPath, "wb");
	if (outputfile == NULL) {
		return 2;
	}

	// -------------- start generation gcode
	gcode_autoUp = true;
	gcode_extrusion = false;

	fprintf(outputfile, "G90\n"); //use absolute coordinates
	fprintf(outputfile, "M83\n"); //extruder relative mode

	if (printer.bedTemperature > 0) {
		fprintf(outputfile, "M140 S%i\n", printer.bedTemperature); //set bed temp
		fprintf(outputfile, "M190 S%i\n", printer.bedTemperature); //wait for bed temp
	}
	bool needDisableTemperature = false;
	if (printer.diskNozzleTemperature > 0) {
		needDisableTemperature = true;
		fprintf(outputfile, "M104 S%i\n", printer.diskNozzleTemperature); //set extruder temp
		fprintf(outputfile, "M109 S%i\n", printer.diskNozzleTemperature); //wait for extruder temp
	}

	fprintf(outputfile, "G28\n");

	// настраиваю
	gcode_fan(outputfile, printer, printer.diskFan);
	gcode_extrusionMultiplier(outputfile, printer, printer.diskExtrusionMultiplier);
	gcode_layerThickness(outputfile, printer, printer.diskLayerThickness);

	// даю экструдеру пропердеться
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 100));
	gcode_dmove(outputfile, printer, 50, 10, 0);
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 10));
	gcode_extrusion = true;
	gcode_move(outputfile, printer, printer.widthX - 50, 10, 0);
	gcode_extrusion = false;

	// начинаю фигачить диск
	gcode_speed(outputfile, printer, util_convertSpeed(printer, printer.fastMoveSpeed));
	double holeRadius = membrane.holeDiameter / 2;
	double diskRadius = membrane.membraneDiameter / 2;
	double zPos = 0;
	bool fromCenter = true;
	for (size_t layer = 0; layer < membrane.membraneLayers; layer++) {
		zPos += printer.diskLayerThickness;
		if (fromCenter) {
			for (double radius = holeRadius; radius <= diskRadius; radius += printer.lineDistance) {
				grammowav_circle(outputfile, printer, zPos, radius, printer.diskPrintSpeed);
			}
		} else {
			for (double radius = diskRadius; radius >= holeRadius; radius -= printer.lineDistance) {
				grammowav_circle(outputfile, printer, zPos, radius, printer.diskPrintSpeed);
			}
		}
		fromCenter = !fromCenter;
		gcode_extrusion = false;
	}

	// меняю настройки на трековые (которые в данном случаи отвечают за поршень на мембране)
	if (printer.trackNozzleTemperature != printer.diskNozzleTemperature && printer.trackNozzleTemperature > 0) {
		needDisableTemperature = true;
		
		gcode_speed(outputfile, printer, util_convertSpeed(printer, printer.fastMoveSpeed));
		gcode_dmove(outputfile, printer, 50, printer.depthY - 10, 10);

		fprintf(outputfile, "M104 S%i\n", printer.trackNozzleTemperature); //set extruder temp
		fprintf(outputfile, "M109 S%i\n", printer.trackNozzleTemperature); //wait for extruder temp
	}
	gcode_fan(outputfile, printer, printer.trackFan);
	gcode_extrusionMultiplier(outputfile, printer, printer.trackExtrusionMultiplier);
	gcode_layerThickness(outputfile, printer, printer.trackLayerThickness);

	// фигачу поршень
	diskRadius = membrane.pistonDiameter / 2;
	double flatLayer = zPos;
	for (size_t layer = 0; layer < membrane.pistonLayers; layer++) {
		zPos += printer.trackLayerThickness;
		if (fromCenter) {
			for (double radius = holeRadius; radius <= diskRadius; radius += printer.lineDistance) {
				grammowav_circle(outputfile, printer, zPos, radius, printer.trackPrintSpeed);
			}
		} else {
			for (double radius = diskRadius; radius >= holeRadius; radius -= printer.lineDistance) {
				grammowav_circle(outputfile, printer, zPos, radius, printer.trackPrintSpeed);
			}
		}
		fromCenter = !fromCenter;
		gcode_extrusion = false;
	}
	gcode_extrusion = false;
	gcode_speed(outputfile, printer, util_convertSpeed(printer, printer.fastMoveSpeed));

	// фигачу уплотнитель
	if (membrane.sealLayers > 0 && membrane.sealSize > 0) {
		zPos = flatLayer;
		diskRadius = membrane.membraneDiameter / 2;
		double radius2 = diskRadius - membrane.sealSize;
		for (size_t layer = 0; layer < membrane.sealLayers; layer++) {
			zPos += printer.trackLayerThickness;
			if (fromCenter) {
				for (double radius = radius2; radius <= diskRadius; radius += printer.lineDistance) {
					grammowav_circle(outputfile, printer, zPos, radius, printer.trackPrintSpeed);
				}
			} else {
				for (double radius = diskRadius; radius >= radius2; radius -= printer.lineDistance) {
					grammowav_circle(outputfile, printer, zPos, radius, printer.trackPrintSpeed);
				}
			}
			fromCenter = !fromCenter;
			gcode_extrusion = false;
		}
	}

	gcode_extrusion = false;
	gcode_speed(outputfile, printer, util_convertSpeed(printer, printer.fastMoveSpeed));
	gcode_dmove(outputfile, printer, 0, printer.depthY, 50);
	gcode_dmove(outputfile, printer, 0, printer.depthY, 0);
	if (printer.bedTemperature > 0) {
		fprintf(outputfile, "M140 S0\n"); //turn off heatbed
	}
	if (needDisableTemperature) {
		fprintf(outputfile, "M104 S0\n"); //turn off temperature
	}
	fprintf(outputfile, "M107\n"); //turn off fan
	fprintf(outputfile, "M84"); //disable motors

	fclose(outputfile);
}