#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include "functions.h"

#define NMAX 200

//urmatoarele functii sunt folosite in mai multe
//comenzi sau in main()

void no_image_loaded(char command[NMAX])
{
	printf("No image loaded\n");
	fgets(command, NMAX, stdin);
}

//aceasta functie verifica daca
//comanda care a fost introdusa de la tastatura
//exista
int valid_command(char *command)
{
	if (strcmp(command, "LOAD") == 0 || strcmp(command, "SELECT") == 0 ||
		strcmp(command, "ROTATE") == 0 || strcmp(command, "CROP") == 0 ||
		strcmp(command, "GRAYSCALE") == 0 || strcmp(command, "SEPIA") == 0 ||
		strcmp(command, "SAVE") == 0) {
		return 1;
	}
	fgets(command, NMAX, stdin);
	return 0;
}

//functia de mai jos ajuta sa tranforme
//sirurile de caractere citite in numere
//de tip intreg
//variabila k verifica daca p apare
//la sfarsitul unei linii (adica retine si \n)
//pentru k = 0 inseamna ca nu apare la sfarsit
//si adauga si ultimul caracter
//daca k > 0 atunci inseamna ca ultimul caracter
//este \n si nu il ia in considerare
int transform_into_number(char *p, int k)
{
	int num = 0;
	int i;

	for (i = 0; i < (int)(strlen(p) - 1); i++)
		num = num * 10 + (p[i] - '0');

	if (k == 0)
		num = num * 10 + (p[strlen(p) - 1] - '0');

	return num;
}

//aceasta functie verifica daca este selectata
//toata imaginea, returnand valoarea 0 in acest caz,
//iar daca nu este schimbata ordinea coordonatelor,
//daca este cazul
int verify_coordinates(coordinates *coord, image_t photo)
{
	if (coord->x1t >= 0 && coord->x1t <= photo.col &&
		coord->x2t >= 0 && coord->x2t <= photo.col &&
		coord->y1t >= 0 && coord->y1t <= photo.lin &&
		coord->y2t >= 0 && coord->y2t <= photo.lin &&
		coord->x2t != coord->x1t && coord->y2t != coord->y1t) {
		if (coord->x1t > coord->x2t) {
			int aux;
			aux = coord->x1t;
			coord->x1t = coord->x2t;
			coord->x2t = aux;
		}
		if (coord->y1t > coord->y2t) {
			int aux;
			aux = coord->y1t;
			coord->y1t = coord->y2t;
			coord->y2t = aux;
		}
		return 1;
	}
	return 0;
}

//aceasta functie este folosita
//pentru GRAYSCALE si SEPIA, rotunjind valorile
//la cel mai apropiat numar intreg
int rounded(double a)
{
	if (2 * (a - (int)a) >= 1)
		return (int)a + 1;
	else
		return (int)a;
}

void initialize_parametres(coordinates *coord)
{
	coord->x1 = -1;
	coord->y1 = -1;
	coord->x2 = -1;
	coord->y2 = -1;
}

//descriere comenzi

//LOAD
//functia LOAD are 2 etape

void read_matrix(image_t *photo, FILE *in)
{
	int i, j;

	for (i = 0; i < photo->lin; i++)
		for (j = 0; j < photo->col; j++)
			fscanf(in, "%d", &photo->image[i][j]);
}

void read_binary_matrix(image_t *photo, long final_byte, FILE *in)
{
	int i, j;

	//sunt citite toate caracterele pana la matrice
	int characters;
	for (i = 0; i < final_byte ; i++)
		fread(&characters, sizeof(char), 1, in);

	//este citita matricea
	for (i = 0 ; i < photo->lin; i++)
		for (j = 0 ; j < photo->col; j++)
			fread(&photo->image[i][j], sizeof(char), 1, in);
}

void skip_comment_lines(FILE *in)
{
	char ign_character;
	while (fscanf(in, "%c", &ign_character) && ign_character == '#') {
		char ign_line[NMAX];
		fgets(ign_line, NMAX, in);
	}
}

//prima este cea in care vede daca exista sau nu fisierul dat
int LOAD(FILE *in, char *filename)
{
	if (!in) {
		printf("Failed to load %s\n", filename);
		return 0;
	}

	printf("Loaded %s\n", filename);
	return 1;
}

void load_c(FILE *in, image_t *photo)
{
	photo->allocated = 0;
	int i;
	if (in) {
		for (i = 0; i < photo->lin; i++)
			free(photo->image[i]);
		free(photo->image);
		fclose(in);
	}
}

//iar a doua retine primele 3 linii din fisier
//prin care se afla tipul fisierului (type+number)
//de forma P1, P2...
//De asemenea, retine numarul de linii si de
//coloane din matricea imagine, numarul maxim
//de pixeli, daca este cazul si matricea imagine
void read_parametres(FILE *in, image_t *photo, char *filename)
{
	char line[NMAX];
	//numeroteaza daca s-au citit toti parametrii doriti
	int par_numb = 0;
	int i;
	photo->max_pixel = 0;
	//sunt citite liniile in parte si sunt atribuite
	//parametrilor valorile
	while (fgets(line, NMAX, in)) {
		//daca linia incepe cu #, inseamna ca
		//este un comentariu care trebuie ignorat,
		if (line[0] == '#')
			continue;

		//se atribuie valorile lui photo.type si photo.number
		if (par_numb == 0) {
			photo->type = line[0];
			photo->number = line[1] - '0';
			par_numb++;
			continue;
		}

		//se retin numarul de linii si de coloane din poza
		if (par_numb == 1) {
			char *p;
			p = strtok(line, " ");
			int last_in_line = 0;

			photo->col = transform_into_number(p, last_in_line);
			p = strtok(NULL, " ");
			last_in_line++;

			photo->lin = transform_into_number(p, last_in_line);
			p = strtok(NULL, " ");
			last_in_line++;

			par_numb++;
			//in cazul in care poza este alb-negru
			//s-a terminat citirea elementelor necesare
			if (photo->number == 1 || photo->number == 4)
				break;
			continue;
		}

		//in celalalt caz trebuie citit si numarul maxim de pixeli
		if (par_numb == 2) {
			photo->max_pixel = transform_into_number(line, 1);
			par_numb++;
			break;
		}
	}

	if (photo->number == 3 || photo->number == 6)
		photo->col = 3 * photo->col;

	//se aloca memorie pentru matricea poza
	photo->image = (int **)calloc(photo->lin, sizeof(int *));
	for (i = 0; i < photo->lin; i++)
		photo->image[i] = (int *)calloc(photo->col, sizeof(int));

	skip_comment_lines(in);

	if (photo->number < 4) {
		//daca fisierul este de tip ascii elementele sunt citite normal
		fseek(in, -1, SEEK_CUR);
		read_matrix(photo, in);
	} else {
		if (photo->number < 7) {
			//se citeste matricea binara
			long final_byte = ftell(in) - 1;
			fclose(in);
			in = fopen(filename, "rb");
			read_binary_matrix(photo, final_byte, in);
		}
	}
}

//SELECT
//Aceasta doar v-a afisa pe ecran daca exista
//coordonatele introduse la tastatura sau nu
//Daca ele nu exista, se vor initializa cu -1 si
//asftel selectia se va face pe toata imaginea

int verify_select(coordinates coord)
{
	if (coord.x1t != -1 && coord.y1t == -1)
		return 0;
	if (coord.y1 != -1 && coord.x2t == -1)
		return 0;
	if (coord.x2 != -1 && coord.y2 == -1)
		return 0;
	return 1;
}

void set_coordinates(coordinates *coord, char *p)
{
	if (p && ((p[0] >= '0' && p[0] <= '9') || p[0] == '-')) {
		coord->x1t = transform_into_number(p, 0);
		p = strtok(NULL, " ");
	} else {
		coord->good = 0;
	}
	if (p && ((p[0] >= '0' && p[0] <= '9') || p[0] == '-')) {
		coord->y1t = transform_into_number(p, 0);
		p = strtok(NULL, " ");
	} else {
		coord->good = 0;
	}
	if (p && ((p[0] >= '0' && p[0] <= '9') || p[0] == '-')) {
		coord->x2t = transform_into_number(p, 0);
		p = strtok(NULL, " ");
	} else {
		coord->good = 0;
	}

	if (p && ((p[0] >= '0' && p[0] <= '9') || p[0] == '-')) {
		coord->y2t = transform_into_number(p, 1);
		p = strtok(NULL, " ");
	} else {
		coord->good = 0;
	}
}

void SELECT(coordinates *coord, image_t photo)
{
	if (photo.number % 3 == 0)
		photo.col = photo.col / 3;

	if (verify_coordinates(coord, photo) == 1) {
		coord->x1 = coord->x1t;
		coord->y1 = coord->y1t;
		coord->x2 = coord->x2t;
		coord->y2 = coord->y2t;
		printf("Selected %d %d %d %d\n", coord->x1,
			   coord->y1, coord->x2, coord->y2);
		if (coord->x1 == 0 && coord->y1 == 0 &&
			coord->x2 == photo.col && coord->y2 == photo.lin) {
			initialize_parametres(coord);
		}
	} else {
		printf("Invalid set of coordinates\n");
	}
}

void select_c(coordinates *coord, image_t photo)
{
	char *all, del[5] = "\n ";
	char *p;
	all = (char *)malloc(NMAX * sizeof(char));
	fgets(all, NMAX, stdin);
	p = strtok(all, del);
	if (strcmp(p, "ALL") == 0) {
		SELECT_ALL(coord);
	} else {
		coord->good = 1;
		set_coordinates(coord, p);
		if (coord->good == 0)
			printf("Invalid command\n");
		else
			SELECT(coord, photo);
	}
	free(all);
}

//SELECT ALL
//Functia initializeaza toate coordonatele cu -1
void SELECT_ALL(coordinates *coord)
{
	initialize_parametres(coord);
	printf("Selected ALL\n");
}

//ROTATE
//Pentru functia rotate e putin mai mult...
//Am facut 2 functii de rotire :
//una care roteste la dreapta toata imaginea
//si una care roteste la stanga o zona patrata
//selectata

//aceasta este functia care roteste la dreapta
//intreaga imagine

void rotate_right_color(image_t *photo)
{
	int i, j, aux;

	color **copie;
	copie = (color **)malloc(photo->col * sizeof(color *));
	for (i = 0; i < photo->col; i++)
		copie[i] = (color *)malloc(photo->lin * sizeof(color));

	//copie va retine imaginea rotita, dar ordinea pixelilor
	//va ramane la fel(rosu, verde si albastru)
	for (i = 0 ; i < photo->lin ; i++)
		for (j = 0 ; j < 3 * photo->col ; j = j + 3) {
			copie[j / 3][photo->lin - i - 1].rosu = photo->image[i][j];
			copie[j / 3][photo->lin - i - 1].verde = photo->image[i][j + 1];
			copie[j / 3][photo->lin - i - 1].albastru =
			photo->image[i][j + 2];
		}

	//se va elibera zona pentru imagine si
	//realoca memorie pentru noua matrice rotita
	for (i = 0; i < photo->lin; i++)
		free(photo->image[i]);
	free(photo->image);

	aux = photo->lin;
	photo->lin = photo->col;
	photo->col = aux;

	photo->image = (int **)calloc(photo->lin, sizeof(int *));
	for (i = 0; i < photo->lin; i++)
		photo->image[i] = (int *)calloc(3 * photo->col, sizeof(int));
	for (i = 0 ; i < photo->lin ; i++)
		for (j = 0 ; j < 3 * photo->col ; j = j + 3) {
			photo->image[i][j] = copie[i][j / 3].rosu;
			photo->image[i][j + 1] = copie[i][j / 3].verde;
			photo->image[i][j + 2] = copie[i][j / 3].albastru;
		}
	photo->col = photo->col * 3;
	for (i = 0; i < photo->lin; i++)
		free(copie[i]);
	free(copie);
}

void rotate_right(image_t *photo)
{
	int i, j, aux;

	int **copie;
	copie = (int **)calloc(photo->col, sizeof(int *));
	for (i = 0; i < photo->col; i++)
		copie[i] = (int *)calloc(photo->lin, sizeof(int));

	for (i = 0 ; i < photo->lin ; i++)
		for (j = 0 ; j < photo->col ; j++)
			copie[j][photo->lin - i - 1] = photo->image[i][j];

	for (i = 0; i < photo->lin; i++)
		free(photo->image[i]);
	free(photo->image);

	aux = photo->lin;
	photo->lin = photo->col;
	photo->col = aux;

	photo->image = (int **)calloc(photo->lin, sizeof(int *));
	for (i = 0; i < photo->lin; i++)
		photo->image[i] = (int *)calloc(photo->col, sizeof(int));

	for (i = 0 ; i < photo->lin ; i++)
		for (j = 0 ; j < photo->col ; j++)
			photo->image[i][j] = copie[i][j];

	for (i = 0; i < photo->lin; i++)
		free(copie[i]);
	free(copie);
}

void rotate_right_all(image_t *photo)
{
	//rotirea se realizeaza cu ajutorul unei copii a imaginii initiale

	if (photo->number == 3 || photo->number == 6) { //pentru imagini color
		photo->col = photo->col / 3;
		rotate_right_color(photo);
	} else {
		rotate_right(photo);
	}
}

//aceasta functie roteste la stanga
//zona patratica de selectie
void rotate_left(image_t *photo, coordinates coord)
{
	int aux;
	int i, j, l;
	//se va proceda la fel ca pentru rotirea intregii
	//imagini, se vor lua 2 cazuri
	if (photo->number % 3 != 0) {
		l = coord.y2 - coord.y1;
		int **copie;
		copie = (int **)calloc(l, sizeof(int *));
		for (i = 0; i < l; i++)
			copie[i] = (int *)calloc(l, sizeof(int));

		for (i = coord.y1; i < coord.y2; i++)
			for (j = coord.x1; j < coord.x2; j++)
				copie[i - coord.y1][j - coord.x1] = photo->image[i][j];

		for (i = 0; i < l / 2; i++)
			for (j = i; j < l - i - 1; j++) {
				aux = copie[i][j];
				copie[i][j] = copie[j][l - i - 1];
				copie[j][l - i - 1] = copie[l - i - 1][l - j - 1];
				copie[l - i - 1][l - j - 1] = copie[l - j - 1][i];
				copie[l - j - 1][i] = aux;
		}

		for (i = coord.y1; i < coord.y2; i++)
			for (j = coord.x1; j < coord.x2; j++)
				photo->image[i][j] = copie[i - coord.y1][j - coord.x1];
		for (i = 0; i < l; i++)
			free(copie[i]);
		free(copie);
	} else {
		//pentru o imagine color trecem zona de selectie
		//intr-o copie de tip color.
		l = coord.y2 - coord.y1;

		color **copie;
		copie = (color **)calloc(l, sizeof(color *));
		for (i = 0; i < l; i++)
			copie[i] = (color *)calloc(l * 3, sizeof(color));

		for (i = coord.y1; i < coord.y2; i++)
			for (j = 3 * coord.x1 ; j < 3 * coord.x2 ; j = j + 3) {
				copie[i - coord.y1][(j - coord.x1 * 3) / 3].rosu =
				photo->image[i][j];
				copie[i - coord.y1][(j - coord.x1 * 3) / 3].verde =
				photo->image[i][j + 1];
				copie[i - coord.y1][(j - coord.x1 * 3) / 3].albastru =
				photo->image[i][j + 2];
			}

		for (i = 0 ; i < l / 2 ; i++)
			for (j = i ; j < l - i - 1; j++) {
				color aux;
				aux = copie[i][j];
				copie[i][j] = copie[j][l - i - 1];
				copie[j][l - 1 - i] = copie[l - 1 - i][l - 1 - j];
				copie[l - 1 - i][l - j - 1] = copie[l - 1 - j][i];
				copie[l - 1 - j][i] = aux;
			}

		for (i = coord.y1; i < coord.y2; i++) {
			for (j = 3 * coord.x1 ; j < 3 * coord.x2 ; j = j + 3) {
				photo->image[i][j] =
				copie[i - coord.y1][(j - coord.x1 * 3) / 3].rosu;
				photo->image[i][j + 1] =
				copie[i - coord.y1][(j - coord.x1 * 3) / 3].verde;
				photo->image[i][j + 2] =
				copie[i - coord.y1][(j - coord.x1 * 3) / 3].albastru;
			}
		}
		for (i = 0; i < l; i++)
			free(copie[i]);
		free(copie);
	}
}

//ROTATE verifica daca unghiul ales este bun
//si in cazul in care este rotita ori o zona patratica
//ori intraga imagine, va apela functiile de mai sus
//de atatea ori incat sa fie rotita la unghiul dorit
int ROTATE(int unghi, coordinates *coord, image_t *photo)
{
	int i;
	if (unghi != 90 && unghi != -90 && unghi != 180 &&
		unghi != -180 && unghi != 270 && unghi != -270 &&
		unghi != 360 && unghi != -360 && unghi != 0)
		return 0;
	if (coord->x1 != -1) {
		if (unghi == 90 || unghi == -270)
			for (i = 0 ; i < 3 ; i++)
				rotate_left(photo, *coord);
		if (unghi == 180 || unghi == -180)
			for (i = 0; i < 2; i++)
				rotate_left(photo, *coord);
		if (unghi == -90 || unghi == 270)
			rotate_left(photo, *coord);
	} else {
		if (unghi == 90 || unghi == -270)
			rotate_right_all(photo);
		if (unghi == 180 || unghi == -180)
			for (i = 0; i < 2; i++)
				rotate_right_all(photo);
		if (unghi == -90 || unghi == 270)
			for (i = 0 ; i < 3 ; i++)
				rotate_right_all(photo);
		initialize_parametres(coord);
	}
	return 1;
}

void rotate_c(coordinates *coord, image_t *photo)
{
	int unghi;
	scanf("%d", &unghi);

	if ((coord->x2 - coord->x1) != (coord->y2 - coord->y1)) {
		printf("The selection must be square\n");
	} else {
		if (ROTATE(unghi, coord, photo) == 0)
			printf("Unsupported rotation angle\n");
		else
			printf("Rotated %d\n", unghi);
	}
}

//CROP
//Functia reinitializeaza numarul de linii si de coloane,
//iar printr-o matrice copie auxiliara, este retinuta zona care
//trebuie taiata, iar, dupa pusa inapoi in
//matricea imagine initiala
void CROP(image_t *photo, coordinates *coord)
{
	int i, j;

	if (photo->number % 3 == 0) {
		//pentru imaginea color este nevoie
		//de o matrice de tip color

		color **copie;
		copie = (color **)calloc((coord->y2 - coord->y1), sizeof(color *));
		for (i = 0; i < (coord->y2 - coord->y1); i++)
			copie[i] = (color *)calloc((coord->x2 - coord->x1), sizeof(color));

		for (i = coord->y1; i < coord->y2; i++)
			for (j = coord->x1 * 3; j < coord->x2 * 3; j = j + 3) {
				copie[i - coord->y1][(j - 3 * coord->x1) / 3].rosu =
				photo->image[i][j];
				copie[i - coord->y1][(j - 3 * coord->x1) / 3].verde =
				photo->image[i][j + 1];
				copie[i - coord->y1][(j - 3 * coord->x1) / 3].albastru =
				photo->image[i][j + 2];
			}

		for (i = 0; i < photo->lin; i++)
			free(photo->image[i]);
		free(photo->image);

		photo->lin = coord->y2 - coord->y1;
		photo->col = 3 * (coord->x2 - coord->x1);

		photo->image = (int **)calloc(photo->lin, sizeof(int *));
		for (i = 0; i < photo->lin; i++)
			photo->image[i] = (int *)calloc(photo->col, sizeof(int));

		for (i = 0; i < photo->lin; i++)
			for (j = 0; j < photo->col; j = j + 3) {
				photo->image[i][j] = copie[i][j / 3].rosu;
				photo->image[i][j + 1] = copie[i][j / 3].verde;
				photo->image[i][j + 2] = copie[i][j / 3].albastru;
			}

		for (i = 0; i < photo->lin; i++)
			free(copie[i]);
		free(copie);

	} else {
		//pentru un alt tip de imagine este
		//folosita o copie de tip int
		int **copie;
		copie = (int **)calloc((coord->y2 - coord->y1), sizeof(int *));
		for (i = 0; i < (coord->y2 - coord->y1); i++)
			copie[i] = (int *)calloc((coord->x2 - coord->x1), sizeof(int));

		for (i = coord->y1; i < coord->y2; i++)
			for (j = coord->x1; j < coord->x2; j++)
				copie[i - coord->y1][j - coord->x1] = photo->image[i][j];

		for (i = 0; i < photo->lin; i++)
			free(photo->image[i]);
		free(photo->image);

		photo->lin = coord->y2 - coord->y1;
		photo->col = coord->x2 - coord->x1;

		photo->image = (int **)calloc(photo->lin, sizeof(int *));
		for (i = 0; i < photo->lin; i++)
			photo->image[i] = (int *)calloc(photo->col, sizeof(int));

		for (i = 0; i < photo->lin; i++)
			for (j = 0; j < photo->col; j++)
				photo->image[i][j] = copie[i][j];

		for (i = 0; i < photo->lin; i++)
			free(copie[i]);
		free(copie);
	}
	initialize_parametres(coord);
}

void crop_c(image_t *photo, coordinates *coord)
{
	if (coord->x1 != -1)
		CROP(photo, coord);
	printf("Image cropped\n");
}

//GRAYSCALE
//Schimba valorile din matricea imagine
//in cele aferente tipului GRAYSCALE
int GRAYSCALE(image_t *photo, coordinates coord)
{
	int i, j;
	if (photo->number != 3 && photo->number != 6)
		return 0;
	if (coord.x1 == -1) {
		coord.x1 = 0;
		coord.y1 = 0;
		coord.x2 = photo->col / 3;
		coord.y2 = photo->lin;
	}
	for (i = coord.y1 ; i < coord.y2; i++)
		for (j = coord.x1 * 3; j < coord.x2 * 3; j = j + 3) {
			double new = (photo->image[i][j] + photo->image[i][j + 1] +
						photo->image[i][j + 2]) / 3;
			int k;
			k = rounded(new);
			photo->image[i][j] = k;
			photo->image[i][j + 1] = k;
			photo->image[i][j + 2] = k;
		}
	return 1;
}

void grayscale_c(image_t *photo, coordinates coord)
{
	if (GRAYSCALE(photo, coord) == 0)
		printf("Grayscale filter not available\n");
	else
		printf("Grayscale filter applied\n");
}

//SEPIA
//Asemenea functiei GRAYSCALE, SEPIA
//schimba anumite valori ai pixelilor
//rosu, verde si albastru
int SEPIA(image_t *photo, coordinates coord)
{
	int i, j;
	if (photo->number != 3 && photo->number != 6)
		return 0;
	if (coord.x1 == -1) {
		coord.x1 = 0;
		coord.y1 = 0;
		coord.y2 = photo->lin;
		coord.x2 = photo->col / 3;
	}

	for (i = coord.y1 ; i < coord.y2; i++)
		for (j = 3 * coord.x1; j < 3 * coord.x2; j = j + 3) {
			double red, green, blue;
			red = (double)photo->image[i][j];
			green = (double)photo->image[i][j + 1];
			blue = (double)photo->image[i][j + 2];

			photo->image[i][j] = rounded((0.393) * red +
										(0.769) * green + (0.189) * blue);
			if (photo->image[i][j] > photo->max_pixel)
				photo->image[i][j] = photo->max_pixel;

			photo->image[i][j + 1] = rounded((0.349) * red +
											(0.686) * green + (0.168) * blue);
			if (photo->image[i][j + 1] > photo->max_pixel)
				photo->image[i][j + 1] = photo->max_pixel;

			photo->image[i][j + 2] = rounded((0.272) * red +
											(0.534) * green + (0.131) * blue);
			if (photo->image[i][j + 2] > photo->max_pixel)
				photo->image[i][j + 2] = photo->max_pixel;
		}
	return 1;
}

void sepia_c(image_t *photo, coordinates coord)
{
	if (SEPIA(photo, coord) == 0)
		printf("Sepia filter not available\n");
	else
		printf("Sepia filter applied\n");
}

//SAVE

void verify_ascii(char *filename, image_t *photo)
{
	//prin aceasta variabila verific daca a
	//fost introdusa si comanda ascii
	char *save_line, del[5] = "\n ";
	save_line = (char *)malloc(NMAX * sizeof(char));
	fgets(save_line, NMAX, stdin);
	char *p = strtok(save_line, del);
	strcpy(filename, p);
	p = strtok(NULL, del);

	//daca nu a fost introdus "ascii"
	//atunci filename va retine si "\n"- ul de la sfarsit
	//care trebuie scos

	if (!p && photo->number < 4)
		photo->number = photo->number + 3;
	//daca apare comanda ascii
	//atunci fisierul trebuie scris sub forma ascii
	//deci se va schimba tipul scris pe prima linie
	//(de la P4 -> P1 , P5 -> P2 si P6 -> P3)
	if (p && strcmp(p, "ascii") == 0 && photo->number > 3) {
		photo->number = photo->number - 3;
		p = strtok(NULL, del);
	}
	free(save_line);
}

//Salveaza imaginea schimbata intr-un fisier
//cu numele introdus la tastatura
void SAVE(FILE *out, char *filename, image_t photo)
{
	out = fopen(filename, "w");

	//pe prima linie apare tipul imaginii (P1, P2, ...)
	fprintf(out, "P%d\n", photo.number);

	//pe a doua numarul de coloane si de linii
	if (photo.number == 3 || photo.number == 6)
		fprintf(out, "%d ", photo.col / 3);
	else
		fprintf(out, "%d ", photo.col);
	fprintf(out, "%d\n", photo.lin);

	//pe a treia numarul maxim de pixeli
	//(daca este cazul)
	if (photo.max_pixel != 0)
		fprintf(out, "%d\n", photo.max_pixel);

	//iar pe urmatoarele linii matricea scrisa
	//ori in binar ori in ascii
	if (photo.number < 4) {
		int i, j;

		for (i = 0; i < photo.lin; i++) {
			for (j = 0; j < photo.col; j++)
				fprintf(out, "%d ", photo.image[i][j]);
			fprintf(out, "\n");
		}
	} else {
		int i, j;
		fclose(out);

		out = fopen(filename, "ab");
		for (i = 0; i < photo.lin; i++)
			for (j = 0; j < photo.col; j++)
				fwrite(&photo.image[i][j], sizeof(char), 1, out);
	}
	fclose(out);
}

void save_c(char *filename, image_t *photo, FILE *out)
{
	verify_ascii(filename, photo);
	SAVE(out, filename, *photo);
	printf("Saved %s\n", filename);
}

//EXIT
//Inchide orice fisier deschis
//Elibereaza zone de memorie
void EXIT(char *filename, image_t *photo, FILE *in,
		  FILE *out, char command[NMAX])
{
	int i;

	free(filename);
	if (photo->allocated == 1) {
		for (i = 0; i < photo->lin; i++)
			free(photo->image[i]);
		free(photo->image);
	}
	if (in)
		fclose(in);
	else
		no_image_loaded(command);
	if (out)
		fclose(out);
}

