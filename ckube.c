/*
 * MIT License
 * ckube.c
 * Copyright (c) 2020 Pablo Peñarroja
 */

#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/*                         */
/*-------- m a t h --------*/
/*                         */

#define M_PI 3.14159265358979323846

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define ABS(x) ((x) < 0 ? -(x) : (x))

int int_random_range(int min_range, int max_range) {
	return (rand() % (max_range - min_range + 1)) + min_range;
}

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

float3 float3_add(float3 l, float3 r) {
	float3 ret = { l.x + r.x, l.y + r.y, l.z + r.z };
	return ret;
}

float3 float3_addf(float3 l, float r) {
	float3 ret =  { l.x + r, l.y + r, l.z + r };
	return ret;
}

float3 float3_sub(float3 l, float3 r) {
	float3 ret = { l.x - r.x, l.y - r.y, l.z - r.z };
	return ret;
}

float3 float3_subf(float3 l, float r) {
	float3 ret = { l.x - r, l.y - r, l.z - r };
	return ret;
}

float3 float3_mult(float3 l, float3 r) {
	float3 ret = { l.x * r.x, l.y * r.y, l.z * l.z };
	return ret;
}

float3 float3_multf(float3 l, float r) {
	float3 ret = { l.x * r, l.y * r, l.z * r };
	return ret;
}

float3 float3_div(float3 l, float3 r) {
	float3 ret = { l.x / r.x, l.y / r.y, l.z / r.z };
	return ret;
}

float3 float3_divf(float3 l, float r) {
	float3 ret = { l.x / r, l.y / r, l.z / r };
	return ret;
}

float float3_length(float3 v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float3 float3_normalize(float3 v) {
	float length = float3_length(v);
	float3 ret = { v.x / length, v.y / length, v.z / length };
	return ret;
}

float float3_dot(float3 l, float3 r) {
	return l.x * r.x + l.y * r.y + l.z * r.z;
}

float3 float3_mod(float3 l, float3 r) {
	float3 ret;
	ret.x = l.x - r.x * floor(l.x / (r.x == 0.0f ? 1.0f : r.x));
	ret.y = l.y - r.y * floor(l.y / (r.y == 0.0f ? 1.0f : r.y));
	ret.z = l.z - r.z * floor(l.z / (r.z == 0.0f ? 1.0f : r.z));
	return ret;
}

float3 float3_abs(float3 v) {
	float3 ret = { ABS(v.x), ABS(v.y), ABS(v.z) };
	return ret;
}

float3 float3_min(float3 l, float3 r) {
	float3 ret = { MIN(l.x, r.x), MIN(l.y, r.y), MIN(l.z, r.z) };
	return ret;
}

float3 float3_max(float3 l, float3 r) {
	float3 ret = { MAX(l.x, r.x), MAX(l.y, r.y), MAX(l.z, r.z) };
	return ret;
}

float3 float3_maxf(float3 l, float r) {
	float3 ret = { MAX(l.x, r), MAX(l.y, r), MAX(l.z, r) };
	return ret;
}

/* multiply by 3x3 matrix */
float3 mult_float3_mat3_3(float3 l, mat3_3 r) {
	return (float3){
		l.x * r.x.x + l.y * r.y.x + l.z * r.z.x,
		l.x * r.x.y + l.y * r.y.y + l.z * r.z.y,
		l.x * r.x.z + l.y * r.y.z + l.z * r.z.z
	};
}

/*                                       */
/*-------- r a y m a r c h i n g --------*/
/*                                       */

float GEOMETRY_SIZE = 1.0f;
float3 GEOMETRY_REPETITION;
float3 HALF_GEOMETRY_REPETITION;

/* cube distance estimator */
float de_cube(float3 point) {
	float3 a = float3_subf(float3_abs(point), GEOMETRY_SIZE);
	return float3_length(float3_maxf(a, 0.0f)) + MIN(MAX(a.x, MAX(a.y, a.z)), 0.0f);
}

/* octahedron distance estimator */
float de_octahedron(float3 point) {
	point = float3_abs(point);
	return (point.x + point.y + point.z - 1.0f) * 0.57735027;
}

/* theoretical modulo infinity */
void infinity_operator(float3* point) {
	*point = float3_sub(float3_mod(float3_add(*point, HALF_GEOMETRY_REPETITION), GEOMETRY_REPETITION), HALF_GEOMETRY_REPETITION);
}

/*                                       */
/*-------- a p p l i c a t i o n --------*/
/*                                       */

void print_help() {
	printf("%s\n", "         _____  __ __  __  __  ___    ____      ");
	printf("%s\n", "        / ___/ / //_/ / / / / / _ )  / __/      ");
	printf("%s\n", "       / /__  /  <   / /_/ / / _  | / _/        ");
	printf("%s\n", "      /____/ /_//_/ /_____/ /____/ /___/        ");
	printf("%s\n", "                                                ");
	printf("%s\n", "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	printf("%s\n", "                                                ");
	printf("%s\n", "-H [int] -> horizontal separation -> 0");
	printf("%s\n", "-V [int] -> vertical separation -> 0");
	printf("%s\n", "-h [int] -> move camera horizontally -> 0");
	printf("%s\n", "-v [int] -> move camera vertically");
	printf("%s\n", "-P [int] -> pitch rotation (deg/frame) -> random");
	printf("%s\n", "-Y [int] -> yaw rotation (deg/frame)   -> random");
	printf("%s\n", "-R [int] -> roll rotation (deg/frame)  -> random");
	printf("%s\n", "                                      ");
	printf("%s\n", "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	printf("%s\n", "                                      ");
	printf("%s\n", "-F [int]   -> frames per second ->  20");
	printf("%s\n", "-f [int]   -> field of view     ->  20");
	printf("%s\n", "-v [float] -> vertical stretch  -> 2.0");
	printf("%s\n", "-s [int]   -> raymarch steps    ->  24");
	printf("%s\n", "-d [float] -> intersect dist    ->1e-3");
	printf("%s\n", "                                      ");
	printf("%s\n", "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

int main(int argc, char *argv[]) {
	print_help();

	/*---- d e c l a r e    v a r s ----*/

	/* application */
	unsigned int run = 1;
	unsigned int cols;
	unsigned int rows;
	unsigned int keypress;
	unsigned int frame_count = 0;
	/* renderer */
	unsigned int fps = 30;
	unsigned int fov = 60;
	unsigned int max_step = 32;
	float min_dist = 2e-3;
	float y_scaling_factor = 2.0f;
	/* scene */
	unsigned int infinity = 0;
	float geometry_repetition_distance = 4.0f;
	float3 geometry_repetition = { 0.0f, 1.0f, 0.0f };
	float camera_distance = 4.0f;
	float camera_movement_x = 0.0f;
	float camera_movement_y = 0.0f;
	int geometry_rotation_x = int_random_range(-5, 5);
	int geometry_rotation_y = int_random_range(-5, 5);
	int geometry_rotation_z = int_random_range(-5, 5);

	/*---- a r g u m e n t s ----*/

	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			print_help();
			return 1;
		}
		switch (argv[i][1]) {
			case 'F':
				fps = atoi(argv[++i]);
				break;
			case 'f':
				fov = atoi(argv[++i]);
				break;
			case 'v':
				y_scaling_factor = (float)atof(argv[++i]);
				break;
			case 's':
				max_step = atoi(argv[++i]);
				break;
			case 'd':
				min_dist = atof(argv[++i]);
				break;
			default:
				print_help();
				return 1;
		}
	}

	/*---- i n i t ----*/

	/* init ncurses */
	initscr();
	noecho();
	curs_set(0);
	timeout(0);

	/* check if terminal supports color */
	if (!has_colors()) {
		printf("%s\n", "[-] Your terminal doesn't support colors. Exiting Program");
		return 1;
	}

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

	/* compute scene after getting fps var */
	geometry_repetition = float3_multf(geometry_repetition, geometry_repetition_distance);
	//camera_movement = float3_divf(camera_movement, fps);

	/* update raymarching global vars */
	GEOMETRY_REPETITION = geometry_repetition;
	HALF_GEOMETRY_REPETITION = float3_divf(geometry_repetition, 2.0f);

	/* configure colors */
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_WHITE, COLOR_BLACK);
	init_pair(3, COLOR_BLUE, COLOR_BLACK);

	/* compute frame duration */
	float time_per_frame = 1.0f / (float)fps;

	/* first frame timestamp */
	clock_t previous_time = clock();

	/*---- m a i n    l o o p ----*/

	for (float3 ori = { 0.0f, 0.0f, camera_distance }; run; ++frame_count) {

		/*---- u s e r    i n p u t ----*/

		/* get out */
		if ((keypress = wgetch(stdscr)) != ERR) {
			switch (keypress) {
				case 27:
				case 'q':
					run = false;
					break;
			}
		}

		/* resolution change */
		int temp_rows;
		int temp_cols;
		getmaxyx(stdscr, temp_rows, temp_cols);
		if (temp_rows != rows || temp_cols != cols) {
			rows = temp_rows;
			cols = temp_cols;
			free(direction_matrix);
			direction_matrix = (float3*)malloc(rows * cols * sizeof(float3));
			/* fill up ray directions matrix */
			for (int r = 0; r < rows; ++r) {
				for (int c = 0; c < cols; ++c) {
					float3 dir;
					dir.y = (float)r * y_scaling_factor + 0.5f - rows * y_scaling_factor / 2.0f;
					dir.x = (float)c + 0.5f - cols / 2.0f;
					dir.z = -(float)rows / tan(fov * M_PI / 180.0f / 2.0f);
					dir = float3_normalize(dir);
					*(direction_matrix + r * cols + c) = dir;
				}
			}
		}

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

		for (int r = 0; r < rows; ++r) {
			for (int c = 0; c < cols; ++c) {
				/* get pixel ray direction */
				float3 dir = *(direction_matrix + r * cols + c);
				float3 point;
				/* raymarch */
				int step = 0;
				for (float total_dist = 0.0f; step < max_step; ++step) {
					/* compute ray point at this step */
					point = float3_add(ori, float3_multf(dir, total_dist));
					/* check if user wants infinity */
					if (infinity & 1) {
						infinity_operator(&point);
					}
					/* apply rotation */
					point = mult_float3_mat3_3(point, general_rotation_matrix);
					/* get distance */
					float dist = de_cube(point);
					if (dist < min_dist) {
						break;
					}
					total_dist += dist;
				}
				/* in case object was hit, draw */
				char draw;
				if (step < max_step) {
					/*---- n o r m a l ----*/
					const float h = 0.001f;
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
					int normal_index = abs((int)normal.x) * 1 + abs((int)normal.y) * 2 + abs((int)normal.z) * 3;
					attron(COLOR_PAIR(normal_index));
					draw = normal_index + 34;
					draw = normal_index == 0 ? ' ' : draw;
				} else {
					draw = ' ';
				}
				mvaddch(r, c, draw);
			}
		}

		/*---- f p s    l i m i t ----*/

		clock_t delta_time = clock() - previous_time;
		clock_t delta_time_sec = (double)(delta_time) / CLOCKS_PER_SEC;
		clock_t time_remaining = (time_per_frame - delta_time_sec) * 1e6;
		if (time_remaining > 0.0f) {
			usleep(time_remaining);
		}
		previous_time = clock();
	}

	/*---- c l e a n u p ----*/

	free(direction_matrix);
	endwin();

	return 0;
}
