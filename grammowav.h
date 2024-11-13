#pragma once

static double convertSample(uint8_t* datapart, size_t size, bool signedInt) {
	switch (size) {
	case 1:
		if (signedInt) {
			return *((int8_t*)datapart) / 128.0;
		}
		else {
			return (*((uint8_t*)datapart) - 127.0) / 128.0;
		}

	case 2:
		if (signedInt) {
			return *((int16_t*)datapart) / 32768.0;
		}
		else {
			return (*((uint16_t*)datapart) - 32767.0) / 32768.0;
		}

	case 4:
		if (signedInt) {
			return *((int32_t*)datapart) / 2147483648.0;
		}
		else {
			return (*((uint32_t*)datapart) - 2147483647.0) / 2147483648.0;
		}
	}
	return 0;
}

int grammowav_wavToGcode(const char* path, const char* exportPath, printer_t printer, disk_t disk) {
	FILE* file = fopen(path, "rb");
	if (file == NULL) return 1;

	FILE* outputfile = fopen(exportPath, "wb");
	if (outputfile == NULL) {
		fclose(file);
		return 2;
	}

	char chunkId[4];
	fread(chunkId, 1, 4, file);

	uint32_t chunkSize;
	fread(&chunkSize, 4, 1, file);

	char format[4];
	fread(format, 1, 4, file);

	char subchunk1Id[4];
	fread(subchunk1Id, 1, 4, file);

	uint32_t subchunk1Size;
	fread(&subchunk1Size, 4, 1, file);

	uint16_t audioFormat;
	fread(&audioFormat, 2, 1, file);

	uint16_t numChannels;
	fread(&numChannels, 2, 1, file);

	uint32_t sampleRate;
	fread(&sampleRate, 4, 1, file);

	uint32_t byteRate;
	fread(&byteRate, 4, 1, file);

	uint16_t blockAlign;
	fread(&blockAlign, 2, 1, file);

	uint16_t bitsPerSample;
	fread(&bitsPerSample, 2, 1, file);

	char subchunk2Id[4];
	fread(&subchunk2Id, 4, 1, file);

	uint32_t offset;
	fread(&offset, 4, 1, file);

	size_t rate = bitsPerSample / 8;
	if (memcmp(subchunk2Id, "data", 4) != 0) {
		fseek(file, offset, SEEK_CUR);
		char title[4];
		fread(&title, 1, 4, file);
		if (memcmp(title, "data", 4) != 0) {
			fclose(file);
			fclose(outputfile);
			return 3;
		}
		fread(&offset, 4, 1, file);
	}
	size_t fileSize = offset;

	// -------------- start generation gcode
	fprintf(outputfile, "G90\n"); //use absolute coordinates
	fprintf(outputfile, "M83\n"); //extruder relative mode

	if (printer.bedTemperature > 0) {
		fprintf(outputfile, "M140 S%i\n", printer.bedTemperature); //set bed temp
		fprintf(outputfile, "M190 S%i\n", printer.bedTemperature); //wait for bed temp
	}
	if (printer.nozzleTemperature > 0) {
		fprintf(outputfile, "M104 S%i\n", printer.nozzleTemperature); //set extruder temp
		fprintf(outputfile, "M109 S%i\n", printer.nozzleTemperature); //wait for extruder temp
	}

	fprintf(outputfile, "G28\n");
	
	//вентилятор на 200
	gcode_fan(outputfile, printer, 200);
	
	// даю экструдеру пропердеться
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 10));
	gcode_move(outputfile, printer, 50, 10, 0);
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 2));
	gcode_extrusion = true;
	gcode_move(outputfile, printer, printer.widthX - 50, 10, 0);
	gcode_extrusion = false;

	// перемещяю башку в центр
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 10));
	gcode_moveC(outputfile, printer, 0, 0, 0);

	// начинаю фигачить диск
	gcode_extrusion = true;
	for (double i = 0; i < 10; i += printer.layerThickness) {
		gcode_moveC(outputfile, printer, 50, 0, i);
		gcode_moveC(outputfile, printer, 50, 50, i);
		gcode_moveC(outputfile, printer, 0, 50, i);
		gcode_moveC(outputfile, printer, 0, 0, i);
	}
	gcode_extrusion = false;

	size_t currentOffset = 0;
	uint8_t datapart[4];
	while (true) {
		double sample = 0;
		for (size_t channel = 0; channel < numChannels; channel++) {
			fread(datapart, 1, rate, file);
			sample += convertSample(datapart, rate, false);
		}
		sample /= numChannels;

		//util_writeMove(outputfile, (printer.widthX / 2) + (sample * 0.1), printer.depthY / 2, 0);

		currentOffset += rate * numChannels;
		if (currentOffset >= fileSize) break;
	}

	gcode_speed(outputfile, printer, util_convertSpeed(printer, 10));
	gcode_move(outputfile, printer, 0, printer.depthY, 50);
	if (printer.bedTemperature > 0) {
		fprintf(outputfile, "M140 S0\n"); //turn off heatbed
	}
	if (printer.nozzleTemperature > 0) {
		fprintf(outputfile, "M104 S0\n"); //turn off temperature
	}
	fprintf(outputfile, "M107\n"); //turn off fan
	fprintf(outputfile, "M84"); //disable motors

	fclose(file);
	fclose(outputfile);

	return 0;
}