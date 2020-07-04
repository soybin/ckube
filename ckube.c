/*
 * MIT License
 * ckube.c
 * Copyright (c) 2020 Pablo Peñarroja
 */

#define _XOPEN_SOURCE_EXTENDED

#include <ncurses.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

/*                         */
/*-------- m a t h --------*/
/*                         */

#define M_PI 3.14159265358979323846

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define ABS(x) ((x) < 0 ? -(x) : (x))

typedef struct _float3 {
	float x;
	float y;
	float z;
} float3;

typedef struct _mat3_3{
	float3 x;
	float3 y;
	float3 z;
} mat3_3;

static inline float3 float3_add(float3 l, float3 r) {
	return (float3){ l.x + r.x, l.y + r.y, l.z + r.z };
}

static inline float3 float3_addf(float3 l, float r) {
	return (float3){ l.x + r, l.y + r, l.z + r };
}

static inline float3 float3_sub(float3 l, float3 r) {
	return (float3){ l.x - r.x, l.y - r.y, l.z - r.z };
}

static inline float3 float3_subf(float3 l, float r) {
	return (float3){ l.x - r, l.y - r, l.z - r };
}

static inline float3 float3_mult(float3 l, float3 r) {
	return (float3){ l.x * r.x, l.y * r.y, l.z * l.z };
}

static inline float3 float3_multf(float3 l, float r) {
	return (float3){ l.x * r, l.y * r, l.z * r };
}

static inline float3 float3_div(float3 l, float3 r) {
	return (float3){ l.x / r.x, l.y / r.y, l.z / r.z };
}

static inline float3 float3_divf(float3 l, float r) {
	return (float3){ l.x / r, l.y / r, l.z / r };
}

static inline float float3_length(float3 v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline float3 float3_normalize(float3 v) {
	float length = float3_length(v);
	float3 ret = { v.x / length, v.y / length, v.z / length };
	return ret;
}

static inline float float3_dot(float3 l, float3 r) {
	return l.x * r.x + l.y * r.y + l.z * r.z;
}

static inline float3 float3_abs(float3 v) {
	float3 ret = { ABS(v.x), ABS(v.y), ABS(v.z) };
	return ret;
}

static inline float3 float3_min(float3 l, float3 r) {
	float3 ret = { MIN(l.x, r.x), MIN(l.y, r.y), MIN(l.z, r.z) };
	return ret;
}

static inline float3 float3_max(float3 l, float3 r) {
	float3 ret = { MAX(l.x, r.x), MAX(l.y, r.y), MAX(l.z, r.z) };
	return ret;
}

static inline float3 float3_maxf(float3 l, float r) {
	float3 ret = { MAX(l.x, r), MAX(l.y, r), MAX(l.z, r) };
	return ret;
}

/* multiply by 3x3 matrix */
static inline float3 float3_mult_mat3_3(float3 l, mat3_3 r) {
	return (float3){
		l.x * r.x.x + l.y * r.y.x + l.z * r.z.x,
		l.x * r.x.y + l.y * r.y.y + l.z * r.z.y,
		l.x * r.x.z + l.y * r.y.z + l.z * r.z.z
	};
}

/* float mod operator */
static inline float float_mod(float l, float r) {
	return l - r * floor(l / (r != 0.0f ? r : 1.0f));
}

/* random integer in range */
static inline int int_random_range(int min_range, int max_range) {
	return (rand() % (max_range - min_range + 1)) + min_range;
}

/* random float in range */
static inline float float_random_range(float min_range, float max_range) {
	return ((float)rand()/(float)(RAND_MAX)) * (max_range - min_range) + min_range;
}

/*                                       */
/*-------- r a y m a r c h i n g --------*/
/*                                       */

/* cube distance estimator */
static inline float de_cube(float3 point) {
	float3 a = float3_subf(float3_abs(point), 1.0f);
	return float3_length(float3_maxf(a, 0.0f)) + MIN(MAX(a.x, MAX(a.y, a.z)), 0.0f);
}

/*                                       */
/*-------- a p p l i c a t i o n --------*/
/*                                       */

void print_help() {
	printf("%s\n", "          _____  __ __  __  __  ___    ____       ");
	printf("%s\n", "         / ___/ / //_/ / / / / / _ )  / __/       ");
	printf("%s\n", "        / /__  /  <   / /_/ / / _  | / _/         ");
	printf("%s\n", "       /____/ /_//_/ /_____/ /____/ /___/         ");
	printf("%s\n", "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	printf("%s\n", "        | press space to pause rendering |        ");
	printf("%s\n", "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	printf("%s\n", "    flag [arg]  |  what is it  |  defaul value    ");
	printf("%s\n", "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	printf("%s\n", "-r         -> random settings            ->  false");
	printf("%s\n", "-c [int]   -> color pallette (0 - 4)     ->      0");
	printf("%s\n", "-1 [int]   -> first unicode render char  ->█(9608)");
	printf("%s\n", "-2 [int]   -> second unicode render char ->█(9608)");
	printf("%s\n", "-3 [int]   -> third unicode render char  ->█(9608)");
	printf("%s\n", "-h         -> print this menu            ->  false");
	printf("%s\n", "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	printf("%s\n", "-H [float] -> horizontal separation      ->    0.0");
	printf("%s\n", "-V [float] -> vertical separation        ->    0.0");
	printf("%s\n", "-m [float] -> move camera horizontally   ->    0.0");
	printf("%s\n", "-M [float] -> move camera vertically     ->    0.0");
	printf("%s\n", "-C [float] -> camera distance in z axis  ->    6.0");
	printf("%s\n", "-P [int]   -> pitch in degrees per frame -> random");
	printf("%s\n", "-Y [int]   -> yaw in degrees per frame   -> random");
	printf("%s\n", "-R [int]   -> roll in degrees per frame  -> random");
	printf("%s\n", "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	printf("%s\n", "-f [int]   -> frames per second          ->     20");
	printf("%s\n", "-F [int]   -> field of view              ->     40");
	printf("%s\n", "-s [float] -> vertical stretch           ->    2.0");
	printf("%s\n", "-S [int]   -> raymarching max steps      ->     32");
	printf("%s\n", "-D [float] -> intersection distance      ->   1e-3");
	printf("%s\n", "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

int main(int argc, char* argv[]) {

	/* allow utf-8 */
	setlocale(LC_ALL, "");

	/* seed random functions */
	srand((unsigned int)time(NULL));

	/*---- d e c l a r e    v a r s ----*/

	/* application */
	unsigned int run = 3; /* first bit -> exit? | second bit -> pause rendering? */
	unsigned int cols;
	unsigned int rows;
	unsigned int keypress;
	unsigned int frame_count = 0;
	/* renderer */
	unsigned int fps = 20;
	unsigned int fov = 40;
	unsigned int max_step = 32;
	float min_dist = 1e-3;
	float y_stretch_factor = 2.0f;
	/* scene */
	int geometry_rotation_x = -1;
	int geometry_rotation_y = -1;
	int geometry_rotation_z = -1;
	int color_one = 1; /* red */
	int color_two = 2; /* green */
	int color_three = 4; /* blue */
	int color_background = 0;
	float geometry_repetition_x = 0.0f;
	float geometry_repetition_y = 0.0f;
	float half_geometry_repetition_x = 0.0f;
	float half_geometry_repetition_y = 0.0f;
	float camera_distance = 6.0f;
	float camera_movement_x = 0.0f;
	float camera_movement_y = 0.0f;
	wchar_t drawing_glyphs[3] = { L'█', L'█', L'█' };

	/*---- a r g u m e n t s ----*/

	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			printf("%s\n", "[-] Invalid argument. Printing argument list.");
			print_help();
			return 1;
		}
		switch (argv[i][1]) {
			case 'r': /* random assignment */
				fov = int_random_range(40, 60);
				if (int_random_range(0, 1)) {
					geometry_repetition_x = float_random_range(4.0f, 6.0f);
					half_geometry_repetition_x = geometry_repetition_x / 2.0f;
					camera_movement_x = float_random_range(-0.1f, 0.1f);
				}
				if (int_random_range(0, 1)) {
					geometry_repetition_y = float_random_range(4.0f, 6.0f);
					half_geometry_repetition_y = geometry_repetition_y / 2.0f;
					camera_movement_y = float_random_range(-0.1f, 0.1f);
				}
				camera_distance = float_random_range(4.0f, 8.0f);
				color_one = int_random_range(1, 7);
				int i;
				for (i = int_random_range(1, 7); i == color_one; i = int_random_range(1, 7));
				color_two = i;
				for (i = int_random_range(1, 7); i == color_one || i == color_two; i = int_random_range(1, 7));
				color_three = i;
				color_background = 0;
				break;
			case 'c': /* color pallette */
				/*
				 * COLOR_BLACK   0
				 * COLOR_RED     1
				 * COLOR_GREEN   2
				 * COLOR_YELLOW  3
				 * COLOR_BLUE    4
				 * COLOR_MAGENTA 5
				 * COLOR_CYAN    6
				 * COLOR_WHITE   7
				 */
				switch (atoi(argv[++i]) % 5) {
					case 1:
						color_one = 3;
						color_two = 5;
						color_three = 6;
						color_background = 0;
						break;
					case 2:
						color_one = 4;
						color_two = 2;
						color_three = 7;
						color_background = 0;
						break;
					case 3:
						color_one = 3;
						color_two = 5;
						color_three = 6;
						break;
					case 4:
						color_one = 7;
						color_two = 7;
						color_three = 7;
						color_background = 0;
				}
				break;
			case '1':
				drawing_glyphs[0] = (wchar_t)atof(argv[++i]);
				break;
			case '2':
				drawing_glyphs[1] = (wchar_t)atoi(argv[++i]);
				break;
			case '3':
				drawing_glyphs[2] = (wchar_t)atoi(argv[++i]);
				break;
			case 'h':
				print_help();
				return 1;
				break;
			case 'H':
				geometry_repetition_x = atof(argv[++i]);
				half_geometry_repetition_x = geometry_repetition_x / 2.0f;
				break;
			case 'V':
				geometry_repetition_y = atof(argv[++i]);
				half_geometry_repetition_y = geometry_repetition_y / 2.0f;
				break;
			case 'm':
				camera_movement_x = atof(argv[++i]);
				break;
			case 'M':
				camera_movement_y = atof(argv[++i]);
				break;
			case 'C':
				camera_distance = atof(argv[++i]);
				break;
			case 'P': /* pitch rotation */
				geometry_rotation_x = atoi(argv[++i]);
				break;
			case 'Y': /* yaw rotation */
				geometry_rotation_y = atoi(argv[++i]);
				break;
			case 'R': /* roll rotation */
				geometry_rotation_z = atoi(argv[++i]);
				break;
			case 'f':
				fps = atoi(argv[++i]);
				break;
			case 'F':
				fov = atoi(argv[++i]);
				break;
			case 's':
				y_stretch_factor = (float)atof(argv[++i]);
				break;
			case 'S':
				max_step = atoi(argv[++i]);
				break;
			case 'D':
				min_dist = atof(argv[++i]);
				break;
			default:
				print_help();
				return 1;
		}
	}

	/* assign rotations if not provided by the user */
	if (geometry_rotation_x < 0 && geometry_rotation_y < 0 && geometry_rotation_z < 0) {
		geometry_rotation_x = int_random_range(0, 5);
		geometry_rotation_y = int_random_range(0, 5);
		geometry_rotation_z = int_random_range(0, 5);
	}

	/*---- i n i t ----*/

	/* init ncurses */
	initscr();
	noecho();
	curs_set(0);
	timeout(0);

	/* check if terminal supports color */
	if (!has_colors()) {
		printf("%s\n", "[-] Your terminal doesn't support colors. Exiting ckube");
		return 1;
	}

	/* configure colors */
	start_color();
	init_pair(1, color_one, color_background);
	init_pair(2, color_two, color_background);
	init_pair(3, color_three, color_background);

	/*
	 * holds the unitary vector direction
	 * for every pixel in the terminal
	 */
	float3* direction_matrix = NULL;

	/*
	 * holds precomputed rotation matrices
	 * for every stage of the cube's rotation.
	 * sin and cos operations are expensive
	 */
	int ratios_x_size = geometry_rotation_x > 0 ? (360 / geometry_rotation_x) : 1;
	int ratios_y_size = geometry_rotation_y > 0 ? (360 / geometry_rotation_y) : 1;
	int ratios_z_size = geometry_rotation_z > 0 ? (360 / geometry_rotation_z) : 1;
	float sin_x[ratios_x_size];
	float cos_x[ratios_x_size];
	float sin_y[ratios_y_size];
	float cos_y[ratios_y_size];
	float sin_z[ratios_z_size];
	float cos_z[ratios_z_size];
	/* pitch */
	for (int i = 0; i < ratios_x_size; ++i) {
		float rotation_x = geometry_rotation_x * i;
		/* to radians */
		rotation_x *= M_PI / 180.0f;
		/* store ratios */
		sin_x[i] = sin(rotation_x);
		cos_x[i] = cos(rotation_x);
	}
	/* yaw */
	for (int i = 0; i < ratios_y_size; ++i) {
		float rotation_y = geometry_rotation_y * i;
		/* to radians */
		rotation_y *= M_PI / 180.0f;
		/* store ratios */
		sin_y[i] = sin(rotation_y);
		cos_y[i] = cos(rotation_y);
	}
	/* roll */
	for (int i = 0; i < ratios_z_size; ++i) {
		float rotation_z = geometry_rotation_z * i;
		/* to radians */
		rotation_z *= M_PI / 180.0f;
		/* store ratios */
		sin_z[i] = sin(rotation_z);
		cos_z[i] = cos(rotation_z);
	}

	/* compute frame duration */
	float time_per_frame = 1.0f / (float)fps;

	/* first frame timestamp */
	clock_t previous_time = clock();

	/*---- m a i n    l o o p ----*/

	for (float3 ori = { 0.0f, 0.0f, camera_distance }; run & (1 << 0); ) {

		/*---- u s e r    i n p u t ----*/

		/* get out */
		if ((keypress = wgetch(stdscr)) != ERR) {
			switch (keypress) {
				case ' ': /* spacebar */
					run ^= (1 << 1);
					break;
				case 27: /* escape */
				case 'q':
					run ^= (1 << 0);
					break;
			}
		}

		/* resolution change */
		int temp_rows;
		int temp_cols;
		getmaxyx(stdscr, temp_rows, temp_cols);
		if (rows != temp_rows || cols != temp_cols) {
			rows = temp_rows;
			cols = temp_cols;
			free(direction_matrix);
			direction_matrix = (float3*)malloc(rows * cols * sizeof(float3));
			/* fill up ray directions matrix */
			for (int r = 0; r < rows; ++r) {
				for (int c = 0; c < cols; ++c) {
					float3 dir;
					dir.y = (float)r * y_stretch_factor + 0.5f - rows * y_stretch_factor / 2.0f;
					dir.x = (float)c + 0.5f - cols / 2.0f;
					dir.z = -(float)rows / tan(fov * M_PI / 180.0f / 2.0f);
					dir = float3_normalize(dir);
					*(direction_matrix + r * cols + c) = dir;
				}
			}
		}

		/* check if rendering is paused */

		if (run & (1 << 1)) {

			/*---- r e n d e r i n g ----*/

			/* update ray origin for this frame */
			ori.x += camera_movement_x;
			ori.y += camera_movement_y;

			/* compute general rotation matrix */
			int pos_x = frame_count % ratios_x_size;
			int pos_y = frame_count % ratios_y_size;
			int pos_z = frame_count % ratios_z_size;
			mat3_3 general_rotation_matrix = (mat3_3) {
				(float3) { cos_z[pos_z] * cos_y[pos_y],
					cos_z[pos_z] * sin_y[pos_y] * sin_x[pos_x] - sin_z[pos_z] * cos_x[pos_x],
					cos_z[pos_z] * sin_y[pos_y] * cos_x[pos_x] + sin_z[pos_z] * sin_x[pos_x] },
				(float3) { sin_z[pos_z] * cos_y[pos_y],
					sin_z[pos_z] * sin_y[pos_y] * sin_x[pos_x] + cos_z[pos_z] * cos_x[pos_x],
					sin_z[pos_z] * sin_y[pos_y] * cos_x[pos_x] - cos_z[pos_z] * sin_x[pos_x] },
				(float3) { -sin_y[pos_y],
					cos_y[pos_y] * sin_x[pos_x],
					cos_y[pos_y] * cos_x[pos_x] }
			};

			int previous_normal_id = 0;
			for (int r = 0; r < rows; ++r) {
				previous_normal_id = 0;
				for (int c = 0; c < cols; ++c) {
					/* get pixel ray direction */
					float3 dir = *(direction_matrix + r * cols + c);
					float3 point;
					/* raymarch */
					int step = 0;
					for (float total_dist = 0.0f; step < max_step; ++step) {
						/* compute intersection */
						point = float3_add(ori, float3_multf(dir, total_dist));
						/* apply infinity using modulo */
						point.x += half_geometry_repetition_x;
						point.y += half_geometry_repetition_y;
						point.x = float_mod(point.x, geometry_repetition_x);
						point.y = float_mod(point.y, geometry_repetition_y);
						point.x -= half_geometry_repetition_x;
						point.y -= half_geometry_repetition_y;
						/* apply rotation */
						point = float3_mult_mat3_3(point, general_rotation_matrix);
						/* get distance */
						float dist = de_cube(point);
						if (dist < min_dist) {
							break;
						}
						total_dist += dist;
					}
					/* in case object was hit, draw */
					wchar_t draw[1] = L" ";
					if (step < max_step) {

						/*---- n o r m a l ----*/

						const float h = 1e-4;
						const float3 xyy = { 1.0f, -1.0f, -1.0f };
						const float3 yyx = { -1.0f, -1.0f, 1.0f };
						const float3 yxy = { -1.0f, 1.0f, -1.0f };
						const float3 xxx = { 1.0f, 1.0f, 1.0f };
						float3 normal = float3_normalize(
								float3_add(
									float3_add(
										float3_multf(xyy, de_cube(float3_add(point, float3_multf(xyy, h)))),
										float3_multf(yyx, de_cube(float3_add(point, float3_multf(yyx, h))))
										),
									float3_add(
										float3_multf(yxy, de_cube(float3_add(point, float3_multf(yxy, h)))),
										float3_multf(xxx, de_cube(float3_add(point, float3_multf(xxx, h))))
										)
									)
								);
						/* get normal id */
						int normal_id = abs((int)normal.x) * 1 + abs((int)normal.y) * 2 + abs((int)normal.z) * 3;
						if (normal_id) {
							attron(COLOR_PAIR(normal_id));
							draw[0] = (wchar_t)drawing_glyphs[normal_id - 1];	
							previous_normal_id = normal_id;
						} else if (previous_normal_id){
							attron(COLOR_PAIR(previous_normal_id));
							draw[0] = (wchar_t)drawing_glyphs[previous_normal_id - 1];	
						}
					}
					/* draw character to screen matrix */
					mvaddwstr(r, c, draw);
				}
			}
			++frame_count;
		}

		/*---- f p s    l i m i t ----*/

		long double delta_time = (long double)(clock() - previous_time) / CLOCKS_PER_SEC;
		long int time_remaining = (time_per_frame - delta_time) * 1e6;
		if (time_remaining > 0) {
			usleep(time_remaining);
		}
		previous_time = clock();
	}

	/*---- c l e a n u p ----*/

	free(direction_matrix);
	endwin();

	return 0;
}
