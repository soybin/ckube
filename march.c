/*
 * MIT License
 * Copyright (c) 2020 Pablo Peñarroja
 */

#include <ncurses.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/*-------- c o n s t s --------*/

const unsigned short SCALING_FACTOR = 1; /* exp */
const unsigned short MAX_DIST = 64;
const unsigned short MAX_STEP = 16;
const unsigned short FPS = 60;
const unsigned short FOV = 45;
const float MIN_DIST = 1e-3;

/*-------- g l o b a l    v a r s --------*/

unsigned int cols;
unsigned int rows;
unsigned long long frame_count;

/*-------- m a t h --------*/

#define M_PI 3.14159265358979323846

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

typedef struct _int3 {
	int x;
	int y;
	int z;
} int3;

typedef struct _float3 {
	float x;
	float y;
	float z;
} float3;

typedef struct _mat4_3{
	int x;
} mat4_3;

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
	ret.x = l.x - r.x * floor(l.x / r.x);
	ret.y = l.y - r.y * floor(l.y / r.y);
	ret.z = l.z - r.z * floor(l.z / r.z);
	return ret;
}

float3 float3_abs(float3 v) {
	float3 ret = { abs(v.x), abs(v.y), abs(v.z) };
}

float3 float3_min(float3 l, float3 r) {
	float3 ret = { MIN(l.x, r.x), MIN(l.y, r.y), MIN(l.z, r.z) };
	return ret;
}

float3 float3_max(float3 l, float3 r) {
	float3 ret = { MAX(l.x, r.x), MAX(l.y, r.y), MAX(l.z, r.z) };
	return ret;
}

/*-------- r a y m a r c h i n g --------*/

float sphere_de(float3 point, float size) {
	float3 q = { 3.0f, 3.0f, 3.0f };
	point = float3_sub(float3_mod(float3_add(point, float3_multf(q, 0.5f)), q), float3_multf(q, 0.5f));
	return float3_length(point) - size;
}

float box_de(float3 point) {
	float3 a = float3_subf(float3_abs(point), 1.0f);
	return float3_length(float3_max(a, (float3){ 0.0f, 0.0f, 0.0f })) + MIN(MAX(a.x, MAX(a.y, a.z)), 0.0f);
}

/*-------- a p p l i c a t i o n --------*/

void update_ray_direction(float3** direction_matrix) {
	free(*direction_matrix);
	*direction_matrix = (float3*)malloc(rows * SCALING_FACTOR * cols * SCALING_FACTOR * sizeof(float3));
	/* fill up */
	for (int r = 0; r < rows * SCALING_FACTOR; ++r) {
		for (int c = 0; c < cols * SCALING_FACTOR; ++c) {
			float3 dir;
			dir.y = (float)r + 0.5f - rows * SCALING_FACTOR / 2.0f;
			dir.x = (float)c + 0.5f - cols * SCALING_FACTOR / 2.0f;
			dir.z = -(float)rows * SCALING_FACTOR / tan(FOV * M_PI / 180.0f / 2.0f);
			dir = float3_normalize(dir);
			*(*direction_matrix + r * cols + c) = dir;
		}
	}
}

int main(int argc, char *argv[]) {
	/* init ncurses */
	initscr();
	noecho();
	curs_set(0);

	/* get window width and height */
	getmaxyx(stdscr, rows, cols);

	/*
	 * holds the unitary vector direction
	 * for every pixel in the terminal
	 */
	float3* direction_matrix = NULL;

	/* initialize pixel directions */
	update_ray_direction(&direction_matrix);

	frame_count = 0;

	float time_per_frame = 1.0f / FPS;
	clock_t previous_time = clock();

	for (;; ++frame_count) {

		/*-------- c h a n g e s --------*/

		int temp_rows;
		int temp_cols;
		getmaxyx(stdscr, temp_rows, temp_cols);
		if (temp_rows != rows || temp_cols != cols) {
			rows = temp_rows;
			cols = temp_cols;
			update_ray_direction(&direction_matrix);
		}

		/*-------- r e n d e r i n g --------*/

		for (int r = 0; r < rows; ++r) {
			for (int c = 0; c < cols; ++c) {
				int step = MAX_STEP;
				for (int i = 0; step == MAX_STEP && i < SCALING_FACTOR; ++i) {
					for (int j = 0; step == MAX_STEP && j < SCALING_FACTOR; ++j) {
						/* camera position */
						float3 ori = { 0.0f, 1.0f, frame_count * 0.1f };
						/* get pixel ray direction */
						float3 dir = *(direction_matrix + r * cols + i * cols + c + j);
						/* raymarch */
						step = 0;
						for (float total_dist = 0.0f; step < MAX_STEP; ++step) {
							float dist = sphere_de(float3_add(ori, float3_multf(dir, total_dist)), 0.25f + frame_count * 0.0001);
							if (dist < MIN_DIST) {
								step = 0;
								break;
							}
							total_dist += dist;
						}
					}
				}
				/* in case object was hit, draw */
				char draw;
				if (step < MAX_STEP) {
					draw = '#';
				} else {
					draw = ' ';
				}
				mvaddch(r, c, draw);			
			}
		}
		refresh();

		/*-------- f p s    l i m i t --------*/

		clock_t delta_time = clock() - previous_time;
		float delta_time_sec = (float)(delta_time) / CLOCKS_PER_SEC;
		float time_remaining = time_per_frame - delta_time_sec;
		if (time_remaining > 0.0f) {
			usleep(time_remaining * 1e6);
		}
		previous_time = clock();
	}

	/*-------- c l e a n u p --------*/

	free(direction_matrix);
	endwin();

	return 0;
}
