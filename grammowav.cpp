#include "microstl.h"

extern "C" int grammowav_wavToStl(const char* path, const char* exportPath, double rpm, double diskSize, double trackWidth, double trackAmplitude) {
	/*
	microstl::MeshWriterHandler handler;
	microstl::MeshProvider provider();
	microstl::Result result = microstl::Writer::writeStlFile(exportPath, provider);
	if (result != microstl::Result::Success) {
		return 1;
	}
	*/

	FILE* file = fopen(path, "rb");
	if (file == NULL) return 1;

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

	uint32_t subchunk2Size;
	fread(&subchunk2Size, 4, 1, file);

	if (byteRate > 4) byteRate = 4;

	size_t currentOffset = 0;
	while (true) {
		uint8_t datapart[4];
		fread(datapart, 1, byteRate, file);
		
		currentOffset += byteRate;
		if (currentOffset >= subchunk2Size) break;
	}

	return 0;
}