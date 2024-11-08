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
	return 0;
}