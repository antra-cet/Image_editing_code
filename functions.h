#pragma once
#define NMAX 200

//Structuri
//color pentru copie
typedef struct {
	int rosu, verde, albastru;
} color;

//image_t pentru photo, ce retine
//tot ce este necesar din fisierul
//citit: tipul si numarul (P1, P2, ...)
//numarul de linii si coloane si matricea poza
typedef struct {
	char type;
	int number;
	int lin, col, max_pixel;
	int **image;
	int allocated;
} image_t;

//coord retine coordonatele zonei selectate sau -1 daca
//se va aplica o comanda pe toata matricea
typedef struct {
	int x1, x2, y1, y2;
	int x1t, x2t, y1t, y2t;
	int good;
} coordinates;

//functiile universale
void no_image_loaded(char command[NMAX]);
int valid_command(char *command);
int verify_coordinates(coordinates *coord, image_t photo);
int transform_into_number(char *p, int k);
int rounded(double a);
void initialize_parametres(coordinates *coord);
//comenzi

//LOAD
void read_matrix(image_t *photo, FILE *in);
void read_binary_matrix(image_t *photo, long final_byte, FILE *in);
int LOAD(FILE *in, char *filename);
void read_parametres(FILE *in, image_t *photo, char *filename);
void load_c(FILE *in, image_t *photo);

//SELECT
int verify_select(coordinates coord);
void set_coordinates(coordinates *coord, char *p);
void SELECT(coordinates *coord, image_t photo);
void select_c(coordinates *coord, image_t photo);

//SELECT ALL
void SELECT_ALL(coordinates *coord);

//ROTATE
void rotate_right(image_t *photo);
void rotate_right_color(image_t *photo);
void rotate_right_all(image_t *photo);
void rotate_left(image_t *photo, coordinates coord);
int ROTATE(int unghi, coordinates *coord, image_t *photo);
void rotate_c(coordinates *coord, image_t *photo);

//CROP
void CROP(image_t *photo, coordinates *coord);
void crop_c(image_t *photo, coordinates *coord);

//GREYSCALE
int GRAYSCALE(image_t *photo, coordinates coord);
void grayscale_c(image_t *photo, coordinates coord);

//SEPIA
int SEPIA(image_t *photo, coordinates coord);
void sepia_c(image_t *photo, coordinates coord);

//SAVE
void verify_ascii(char *filename, image_t *photo);
void SAVE(FILE *out, char *filename, image_t photo);
void save_c(char *filename, image_t *photo, FILE *out);

//EXIT
void EXIT(char *filename, image_t *photo, FILE *in,
		  FILE *out, char command[NMAX]);
