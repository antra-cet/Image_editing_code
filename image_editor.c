#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include "functions.h"

#define NMAX 200

int main(void)
{
	char command[NMAX];
	char *filename;
	filename = (char *)malloc(sizeof(char) * NMAX);
	FILE *in = NULL;
	FILE *out = NULL;
	image_t photo;
	coordinates coord;
	photo.allocated = 0;
	initialize_parametres(&coord);

	while (scanf("%s", command) && strcmp(command, "EXIT") != 0) {
		if (valid_command(command) == 0)
			printf("Invalid command\n");
		//LOAD
		if (strcmp(command, "LOAD") == 0) {
			load_c(in, &photo);

			scanf("%s", filename);
			in = fopen(filename, "r");
			if (LOAD(in, filename) == 1) {
				read_parametres(in, &photo, filename);
				photo.allocated = 1;
			}

			initialize_parametres(&coord);
		}
		//SELECT
		if (strcmp(command, "SELECT") == 0) {
			if (!in)
				no_image_loaded(command);
			else
				select_c(&coord, photo);
		}
		//ROTATE
		if (strcmp(command, "ROTATE") == 0) {
			if (!in)
				no_image_loaded(command);
			else
				rotate_c(&coord, &photo);
		}
		//CROP
		if (strcmp(command, "CROP") == 0) {
			if (!in)
				no_image_loaded(command);
			else
				crop_c(&photo, &coord);
		}
		//GRAYSCALE
		if (strcmp(command, "GRAYSCALE") == 0) {
			if (!in)
				no_image_loaded(command);
			else
				grayscale_c(&photo, coord);
		}
		//SEPIA
		if (strcmp(command, "SEPIA") == 0) {
			if (!in)
				no_image_loaded(command);
			else
				sepia_c(&photo, coord);
		}
		//SAVE
		if (strcmp(command, "SAVE") == 0) {
			if (!in) {
				no_image_loaded(command);
			} else {
				out = NULL;
				save_c(filename, &photo, out);
			}
		}
		//EXIT
		if (strcmp(command, "EXIT") == 0)
			break;
	}

	EXIT(filename, &photo, in, out, command);
	return 0;
}
