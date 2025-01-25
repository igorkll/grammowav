#pragma once

int membrane_generate(const char* exportPath, printer_t printer, membrane_t membrane) {
	FILE* outputfile = fopen(exportPath, "wb");
	if (outputfile == NULL) {
		return 2;
	}

	fclose(outputfile);
}