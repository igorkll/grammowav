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

void grammowav_debugExportWav(double* samples, size_t samplesCount, uint32_t sampleRate) {
	FILE* outputfile = fopen("D:\\debug.wav", "wb");
	fwrite("RIFF", 1, 4, outputfile);
	uint32_t chunkSize = (samplesCount * sizeof(uint32_t)) + (44 - 8);
	fwrite(&chunkSize, sizeof(uint32_t), 1, outputfile);
	fwrite("WAVE", 1, 4, outputfile);
	fwrite("fmt ", 1, 4, outputfile);
	uint32_t subchunk1Size = 16;
	fwrite(&subchunk1Size, sizeof(uint32_t), 1, outputfile);
	uint16_t audioFormat = 1;
	fwrite(&audioFormat, sizeof(uint16_t), 1, outputfile);
	uint16_t numChannels = 1;
	fwrite(&numChannels, sizeof(uint16_t), 1, outputfile);
	fwrite(&sampleRate, sizeof(uint32_t), 1, outputfile);
	uint32_t byteRate = sampleRate * sizeof(uint32_t);
	fwrite(&byteRate, sizeof(uint32_t), 1, outputfile);
	uint16_t blockAlign = sizeof(uint32_t);
	fwrite(&blockAlign, sizeof(uint16_t), 1, outputfile);
	uint16_t bitsPerSample = 32;
	fwrite(&bitsPerSample, sizeof(uint16_t), 1, outputfile);
	fwrite("data", 1, 4, outputfile);
	uint32_t subchunk2Size = samplesCount * sizeof(uint32_t);
	fwrite(&subchunk2Size, sizeof(uint32_t), 1, outputfile);
	for (size_t i = 0; i < samplesCount; i++) {
		int32_t sample = samples[i] * 2147483647;
		fwrite(&sample, sizeof(uint32_t), 1, outputfile);
	}
	fclose(outputfile);
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
	size_t realSamplesCount = fileSize / rate / numChannels;

	// -------------- start generation gcode
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
	gcode_layerThickness(outputfile, printer, printer.layerThickness);
	
	// даю экструдеру пропердеться
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 100));
	gcode_dmove(outputfile, printer, 50, 10, 0);
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 10));
	gcode_extrusion = true;
	gcode_move(outputfile, printer, printer.widthX - 50, 10, 0);
	gcode_extrusion = false;

	// начинаю фигачить диск
	gcode_speed(outputfile, printer, util_convertSpeed(printer, printer.fastMoveSpeed));
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
	gcode_speed(outputfile, printer, util_convertSpeed(printer, printer.fastMoveSpeed));
	gcode_moveC(outputfile, printer, 0, 0, 50);
	gcode_dmove(outputfile, printer, 50, printer.depthY - 10, 0);

	// читаю ВЕСЬ wav в оперативу (сам знаю что дофига весить будет, но мне сейчас не до оптимизации)
	double* soundData = malloc(realSamplesCount * sizeof(double));
	if (soundData == NULL) {
		fclose(outputfile);
		fclose(file);
		return 4;
	}
	size_t currentSample = 0;
	uint8_t datapart[4];
	bool signedPcm = rate > 1;
	while (true) {
		double sample = 0;
		for (size_t channel = 0; channel < numChannels; channel++) {
			fread(datapart, 1, rate, file);
			sample += convertSample(datapart, rate, signedPcm);
		}

		soundData[currentSample] = sample / numChannels;
		currentSample++;
		if (currentSample >= realSamplesCount) break;
	}
	double numberSamplesPerturn = sampleRate / (disk.rpm / 60);
	size_t emptyTrack = numberSamplesPerturn * 2;
	size_t samplesCount = realSamplesCount + emptyTrack;

	// нармализую звук, деля его на фрагменты а потом подбирая множитель пока не упреться в предел
	if (disk.normalizeSound) {
		int normalize_frame = sampleRate / 8;
		double normalize_step = 0.05;
		double normalize_minProcessValue = 0.01;
		double normalize_mulUp = 1 + normalize_step;
		double normalize_mulDown = 1 - normalize_step;

		for (size_t offset = 0; offset < realSamplesCount - normalize_frame; offset += normalize_frame) {
			bool soundexists = false;
			for (size_t frameOffset = 0; frameOffset < normalize_frame; frameOffset++) {
				double value = soundData[offset + frameOffset];
				if (value > normalize_minProcessValue || value < -normalize_minProcessValue) soundexists = true;
			}
			while (soundexists) {
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
		grammowav_debugExportWav(soundData, realSamplesCount, sampleRate);
	}

	// меняю настройки на трековые
	if (printer.trackNozzleTemperature != printer.diskNozzleTemperature && printer.trackNozzleTemperature > 0) {
		needDisableTemperature = true;
		fprintf(outputfile, "M104 S%i\n", printer.trackNozzleTemperature); //set extruder temp
		fprintf(outputfile, "M109 S%i\n", printer.trackNozzleTemperature); //wait for extruder temp
	}
	gcode_fan(outputfile, printer, printer.trackFan);
	gcode_extrusionMultiplier(outputfile, printer, printer.trackExtrusionMultiplier);

	// стираю излишки пластика перед печатью трека, но с другой стороны(потому что на прошлой уже насрано экструзией)
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 10));
	gcode_dmove(outputfile, printer, printer.widthX - 50, printer.depthY - 10, 0);

	// фигачу дорожку
	gcode_speed(outputfile, printer, util_convertSpeed(printer, 100));
	double labelRadius = disk.labelDiameter / 2;
	double trackOffset = (diskRadius - labelRadius) / samplesCount;
	double trackOffsetPerturn = trackOffset * numberSamplesPerturn;
	double minTrackOffsetPerturn;
	if (disk.matrix) {
		minTrackOffsetPerturn = printer.nozzleDiameter + (disk.trackAmplitude * 2);
	} else {
		minTrackOffsetPerturn = disk.trackWidth + (printer.nozzleDiameter / 2) + disk.trackAmplitude;
	}
	if (trackOffsetPerturn < minTrackOffsetPerturn) {
		fclose(outputfile);
		fclose(file);
		return 5;
	}
	while (true) {
		for (uint8_t n = 1; n <= (disk.matrix ? 1 : 2); n++) {
			currentSample = 0;
			double radius = diskRadius - (disk.trackWidth * n);
			while (true) {
				double sample = 0;
				if (currentSample >= emptyTrack)
					sample = soundData[currentSample - emptyTrack];

				double rotate = (((double)currentSample) / numberSamplesPerturn) * M_PI * 2;
				if (disk.matrix) rotate = -rotate;
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
	gcode_speed(outputfile, printer, util_convertSpeed(printer, printer.fastMoveSpeed));
	gcode_moveC(outputfile, printer, 0, 0, 50);
	gcode_move(outputfile, printer, 0, printer.depthY, 0);
	if (printer.bedTemperature > 0) {
		fprintf(outputfile, "M140 S0\n"); //turn off heatbed
	}
	if (needDisableTemperature) {
		fprintf(outputfile, "M104 S0\n"); //turn off temperature
	}
	fprintf(outputfile, "M107\n"); //turn off fan
	fprintf(outputfile, "M84"); //disable motors

	free(soundData);
	fclose(file);
	fclose(outputfile);

	return 0;
}