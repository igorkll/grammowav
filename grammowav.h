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

typedef struct {
	int nozzleTemperature;
	int bedTemperature;

	int widthX;
	int depthY;
	int heightZ;
	double zOffset;

	double nozzleDiameter;
	double filamentDiameter;

	char start_gcode[32 * 1024];
	char end_gcode[32 * 1024];
} printer_t;

typedef struct {
	double rpm;
	double diskSize;
	double trackWidth;
	double trackAmplitude;
} disk_t;

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
	util_writeln(outputfile, "G90"); //use absolute coordinates
	util_writeln(outputfile, "M83"); //extruder relative mode

	util_write(outputfile, "M140 S"); //set bed temp
	util_writeNumberln(outputfile, printer.bedTemperature);
	util_write(outputfile, "M190 S"); //wait for bed temp
	util_writeNumberln(outputfile, printer.bedTemperature);

	util_write(outputfile, "M104 S"); //set extruder temp
	util_writeNumberln(outputfile, printer.nozzleTemperature);
	util_write(outputfile, "M109 S"); //wait for extruder temp
	util_writeNumberln(outputfile, printer.nozzleTemperature);

	util_writeln(outputfile, "G28");

	size_t currentOffset = 0;
	uint8_t datapart[4];
	while (true) {
		double sample = 0;
		for (size_t channel = 0; channel < numChannels; channel++) {
			fread(datapart, 1, rate, file);
			sample += convertSample(datapart, rate, false);
		}
		sample /= numChannels;

		char buffer[32];
		_itoa(((sample + 1.0) / 2.0) * 255, buffer, 10);
		//fwrite(buffer, 1, strlen(buffer), outputfile);

		currentOffset += rate * numChannels;
		if (currentOffset >= fileSize) break;
	}

	util_writeln(outputfile, "G1 Z4 F600"); //Move print head up
	util_writeln(outputfile, "G1 X5 Y209 F18000"); //present print
	util_writeln(outputfile, "G1 Z72 F600"); //Move print head further up
	util_writeln(outputfile, "G1 Z150 F600"); //Move print head further up
	util_writeln(outputfile, "M140 S0"); //turn off heatbed
	util_writeln(outputfile, "M104 S0"); //turn off temperature
	util_writeln(outputfile, "M107"); //turn off fan
	util_writeln(outputfile, "M84"); //disable motors

	fclose(file);
	fclose(outputfile);

	return 0;
}