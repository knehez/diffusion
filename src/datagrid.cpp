#include <stdlib.h>
#include <stdio.h>
#include "datagrid.h"


inline void numset(double arr[], double val, size_t count) {
	for (size_t i = 0; i < count; ++i)
		arr[i] = val;
}

DIFFUSIONLIB_API bool allocate(Datagrid& grid, const Scaling& scaling) {
	grid.data = (double*)calloc(scaling.nr_tot(), sizeof(double));
	if (grid.data) {
		grid.scaling = scaling;
		return false;
	}
	else {
		grid.scaling.clear();
		return true;
	}
}


DIFFUSIONLIB_API void release(Datagrid& grid) {
	if (grid.data)
		free(grid.data);
	grid.data = NULL;
	grid.scaling.clear();
}


DIFFUSIONLIB_API int load(const char* filename, Datagrid &grid) {
	// load-logic here!
	int retval = 0;
	long filelen = 0;
	char bom[3] = { '\0', '\0', '\0'};
	size_t n;

	// fopen
	FILE *file;
	errno_t err = fopen_s(&file, filename, "rb");
	if (err || !file) {
		retval = 1;  // unable to fopen
		goto fin;
	}

	// check size
	fseek(file, 0L, SEEK_END);
	filelen = ftell(file);
	if (filelen < 2 * sizeof(char) + 2 * sizeof(size_t) + 2 * sizeof(double)) {
		retval = 3;  // file too short
		goto fin;
	}
	fseek(file, 0L, SEEK_SET);

	// read BOM
	if (fread(&bom, sizeof(char), 2, file) < 2 || strcmp(bom, "KD")) {
		retval = 4;  // further investigation required
		goto fin;
	}

	// read DIM
	if (fread(&grid.scaling._dim, sizeof(size_t), 1, file) < 1 || grid.scaling._dim == 0 || grid.scaling._dim > 3) {
		retval = 4;  // further investigation required
		goto fin;
	}

	// read N
	grid.scaling._nr[0] = 1; grid.scaling._nr[1] = 1; grid.scaling._nr[2] = 1;
	if (fread(grid.scaling._nr, sizeof(size_t), grid.scaling._dim, file) < grid.scaling._dim) {
		retval = 4;  // further investigation required
		goto fin;
	}
	n = grid.scaling.nr_tot();

	// read R_init
	if (fread(grid.scaling._r_min, sizeof(double), grid.scaling._dim, file) < grid.scaling._dim) {
		retval = 4;  // further investigation required
		goto fin;
	}

	// read R_fin
	if (fread(grid.scaling._r_max, sizeof(double), grid.scaling._dim, file) < grid.scaling._dim /*|| r_fin[0] <= r_init[0] || r_fin[1] <= r_init[1] || r_fin[2] <= r_init[2]*/) {
		retval = 4;  // further investigation required
		goto fin;
	}

	// check remaining size
	if (filelen - ftell(file) < n * sizeof(double)) {
		retval = 3;  // file too short
		goto fin;
	}

	// allocate memory
	grid.data = (double*) calloc(n, sizeof(double));
	if (!grid.data) {
		retval = 6;  // out of memory
		goto fin;
	}

	// read data
	if (fread(grid.data, sizeof(double), n, file) < n) {
		retval = 4;  // further investigation required
		goto fin;
	}

	// check position: 3 file too short, 5 file too long, 0 OK
	retval = ftell(file) < filelen ? 3 : ftell(file) > filelen ? 5 : 0;

fin:
	if (retval) {  // reading or parsing unsuccessfull
		if (grid.data)
			free(grid.data);
		grid.scaling.clear();
	}
	if (retval == 4) {  // further investigation: 2 fread error, 3 file too short, 4 simply rubbish data
		retval = ferror(file) ? 2 : feof(file) ? 3 : 4;
	}
	if (file) {  // close file
		fclose(file);
		file = NULL;
	}

	return retval;
}


DIFFUSIONLIB_API int save(const char *filename, const Datagrid &grid) {
	// save-logic here!
	int retval = 0;
	size_t n = grid.scaling.nr_tot();

	// fopen
	FILE *file;
	errno_t err = fopen_s(&file, filename, "wb");
	if (err || !file)
		retval = 1;  // unable to fopen
	else if (
			fwrite("KD", sizeof(char), 2, file) < 2 ||
			fwrite(&grid.scaling._dim, sizeof(size_t), 1, file) < 1 ||
			fwrite(grid.scaling._nr, sizeof(size_t), grid.scaling._dim, file) < grid.scaling._dim ||
			fwrite(grid.scaling._r_min, sizeof(double), grid.scaling._dim, file) < grid.scaling._dim ||
			fwrite(grid.scaling._r_max, sizeof(double), grid.scaling._dim, file) < grid.scaling._dim ||
			fwrite(grid.data, sizeof(double), n, file) < n
			)
		retval = 2;
	else
		retval = 0;
	if (file)
		fclose(file);
	return retval;
}

