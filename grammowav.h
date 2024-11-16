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
	size_t samplesCount = fileSize / rate / numChannels;

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
	gcode_fan(outputfile, printer, printer.fan);
	
	// даю экструдеру пропердеться
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 100));
	gcode_dmove(outputfile, printer, 50, 10, 0);
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 10));
	gcode_extrusion = true;
	gcode_move(outputfile, printer, printer.widthX - 50, 10, 0);
	gcode_extrusion = false;

	// начинаю фигачить диск
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 100));
	double holeRadius = disk.holeDiameter / 2;
	double diskRadius = disk.diskDiameter / 2;
	double zPos = 0;
	while (true) {
		for (double radius = diskRadius; radius > holeRadius; radius -= printer.lineDistance) {
			for (size_t i = 0; i < printer.circleFacesNumber; i++) {
				double rotate = (((double)i) / ((double)(printer.circleFacesNumber - 1))) * M_PI * 2;
				gcode_moveC(outputfile, printer, sin(rotate) * radius, cos(rotate) * -radius, zPos);
				if (!gcode_extrusion) {
					gcode_speed(outputfile, printer, util_convertSpeed(printer, printer.diskPrintSpeed));
					gcode_extrusion = true;
				}
			}
		}
		gcode_extrusion = false;
		zPos += printer.layerThickness;
		if (zPos >= disk.diskHeight - disk.trackHeight) {
			break;
		}
	}
	gcode_extrusion = false;

	// читаю ВЕСЬ wav в оперативу (сам знаю что дофига весить будет, но мне сейчас не до оптимизации)
	double* soundData = malloc(samplesCount * sizeof(double));
	size_t currentSample = 0;
	uint8_t datapart[4];
	while (true) {
		double sample = 0;
		for (size_t channel = 0; channel < numChannels; channel++) {
			fread(datapart, 1, rate, file);
			sample += convertSample(datapart, rate, false);
		}

		soundData[currentSample] = sample / numChannels;
		currentSample++;
		if (currentSample >= samplesCount) break;
	}

	// нармализую звук, деля его на фрагменты а потом подбирая множитель пока не упреться в предел
	int normalize_frame = sampleRate / 8;
	double normalize_step = 0.05;
	double normalize_mulUp = 1 + normalize_step;
	double normalize_mulDown = 1 - normalize_step;

	for (size_t offset = 0; offset < samplesCount; offset += normalize_frame) {
		while (true) {
			bool overrange = false;
			for (size_t frameOffset = 0; frameOffset < normalize_frame; frameOffset++) {
				soundData[offset + frameOffset] *= normalize_mulUp;
				double value = soundData[offset + frameOffset];
				if (value > 1 || value < -1) overrange = true;
			}
			if (overrange) {
				for (size_t frameOffset = 0; frameOffset < normalize_frame; frameOffset++) {
					soundData[offset + frameOffset] *= normalize_mulDown;
				}
				break;
			}
		}
	}
	
	// фигачу дорожку
	double numberSamplesPerturn = sampleRate / (disk.rpm / 60);
	double labelRadius = disk.labelDiameter / 2;
	double trackOffset = (diskRadius - labelRadius) / samplesCount;
	while (true) {
		for (uint8_t n = 1; n <= 2; n++) {
			currentSample = 0;
			double radius = diskRadius - (disk.trackWidth * n);
			while (true) {
				double sample = soundData[currentSample];

				double rotate = (((double)currentSample) / numberSamplesPerturn) * M_PI * 2;
				double localRadius = radius - (sample * disk.trackAmplitude);
				gcode_moveC(outputfile, printer, sin(rotate) * localRadius, cos(rotate) * -localRadius, zPos);
				if (!gcode_extrusion) {
					gcode_speed(outputfile, printer, util_convertSpeed(printer, printer.trackPrintSpeed));
					gcode_extrusion = true;
				}

				radius -= trackOffset;
				currentSample++;
				if (currentSample >= samplesCount) break;
			}
			gcode_extrusion = false;
		}
		zPos += printer.layerThickness;
		if (zPos >= disk.diskHeight) {
			break;
		}
	}

	gcode_extrusion = false;
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 100));
	gcode_moveC(outputfile, printer, 0, 0, 50);
	if (printer.bedTemperature > 0) {
		fprintf(outputfile, "M140 S0\n"); //turn off heatbed
	}
	if (printer.nozzleTemperature > 0) {
		fprintf(outputfile, "M104 S0\n"); //turn off temperature
	}
	fprintf(outputfile, "M107\n"); //turn off fan
	fprintf(outputfile, "M84"); //disable motors

	free(soundData);
	fclose(file);
	fclose(outputfile);

	return 0;
}