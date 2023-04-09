#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ROWS 7
#define COLS 9
#define SHIPS 4

struct point {
	unsigned int x;
	unsigned int y;
};

enum result {
	NOT_SCANNED = '_',
	NOTHING = '*',
	SEE_0 = '0',
	SEE_1 = '1',
	SEE_2 = '2',
	SEE_3 = '3',
	SEE_4 = '4',
	SHIP = '@'
};

enum result field[ROWS*COLS];
struct point ships[SHIPS];
unsigned int hits;

static void print_field(void)
{
	unsigned int x, y;
	printf("  1 2 3 4 5 6 7 8 9\n");

	for (y = 0; y < ROWS; ++y) {
		printf("%c", 'A' + y);
		for (x = 0; x < COLS; ++x)
			printf(" %c", field[y*COLS + x]);
		printf("\n");
	}
}

static void init_ships(void)
{
	unsigned int k;
	unsigned int filled = 0;

	while (filled < SHIPS) {
		unsigned int x = rand() % COLS;
		unsigned int y = rand() % ROWS;
		bool invalid = false;

		/* make sure, that no ships have the same position */
		for (k = 0; k < filled; ++k) {
			if ((ships[k].x == x) && (ships[k].y == y)) {
				invalid = true;
				break;
			}
		}
		if (invalid)
			continue;

		ships[k].x = x;
		ships[k].y = y;
		filled++;
	}
}

enum heading {
	HEADING_NW,
	HEADING_N,
	HEADING_NE,
	HEADING_W,
	HEADING_E,
	HEADING_SE,
	HEADING_S,
	HEADING_SW,
	HEADING_IDENT,
	HEADING_MAX
};

/* find in which heading a ship is seen from the scan position */
static enum heading get_heading(unsigned int x_scan, unsigned int y_scan, unsigned int x_ship, unsigned int y_ship)
{
	if ((x_ship < x_scan) && (y_ship < y_scan))
		return HEADING_NW;
	else if ((x_ship < x_scan) && (y_ship == y_scan))
		return HEADING_W;
	else if ((x_ship < x_scan) && (y_ship > y_scan))
		return HEADING_SW;
	else if ((x_ship == x_scan) && (y_ship < y_scan))
		return HEADING_N;
	else if ((x_ship == x_scan) && (y_ship == y_scan))
		return HEADING_IDENT;
	else if ((x_ship == x_scan) && (y_ship > y_scan))
		return HEADING_S;
	if ((x_ship > x_scan) && (y_ship < y_scan))
		return HEADING_NE;
	else if ((x_ship > x_scan) && (y_ship == y_scan))
		return HEADING_E;
	else if ((x_ship > x_scan) && (y_ship > y_scan))
		return HEADING_SE;
}

static void read_target(unsigned int *x, unsigned int *y)
{
	printf("TARGET %u/%u: ", hits, SHIPS);
	while (1) {
		char c = getchar();
		if ((c >= 'A') && (c < 'A' + ROWS)) {
			*y = c - 'A';
			break;
		}
		if ((c >= 'a') && (c < 'a' + ROWS)) {
			*y = c - 'a';
			break;
		}
	}
	while (1) {
		char c = getchar();
		if ((c >= '1') && (c < '1' + COLS)) {
			*x = c - '1';
			break;
		}
	}
}

/* scan from point x, y */
static void scan(unsigned int x, unsigned int y)
{
	enum result res = SEE_0;
	unsigned int headings[HEADING_MAX] = { 0 };;

	for (unsigned int k = 0; k < SHIPS; ++k) {
		/* check for direct hit */
		if ((x == ships[k].x) && (y == ships[k].y)) {
			res = SHIP;
			break;
		} else if ((x == ships[k].x) || (y == ships[k].y) /* ship in same col/row */
					|| (abs(x - ships[k].x) == abs(y - ships[k].y))) { /*ship on diagon */
			headings[get_heading(x, y, ships[k].x, ships[k].y)]++; /* asign sighting to a heading */
		}
	}
	if (res == SEE_0) {
#if 0
		/* print sightings/heading */
		printf("%u %u %u\n", headings[HEADING_NW], headings[HEADING_N], headings[HEADING_NE]);
		printf("%u %u %u\n", headings[HEADING_W], headings[HEADING_IDENT], headings[HEADING_E]);
		printf("%u %u %u\n", headings[HEADING_SW], headings[HEADING_S], headings[HEADING_SE]);
#endif
		/* count sightings for each heading only once, because ships cover up each other */
		for (unsigned int k = 0; k < HEADING_MAX; ++k)
			if (headings[k])
				res++;
	}
	/* in case of zero sightings mark all places where no ship can be */
	if (res == SEE_0) {
		for (unsigned int k = 0; k < ROWS*COLS; ++k) {
			unsigned int x0 = k % COLS;
			unsigned int y0 = k / COLS;

			if (((x == x0) || (y == y0) || (abs(x - x0) == abs(y - y0))) && (field[k] == NOT_SCANNED))
				field[k] = NOTHING;
		}
	}
	if ((res == SHIP) && (field[y * COLS + x] != SHIP))
		hits++;
	field[y * COLS + x] = res;
#if 0
	printf("scan %c%c: field[%u] = %c\n", 'A'+ y, '0' + x, y * COLS + x, res);
#endif
}

int main(void)
{
	unsigned int x, y;

	/* setup random generator */
	srand(time(NULL));

	/* init field */
	for (unsigned int k =0; k < ROWS*COLS; ++k)
		field[k] = NOT_SCANNED;

	init_ships();
#if 0
	/* dump ship positions */
	for (unsigned int k = 0; k < SHIPS; ++k)
		printf("%u: %c%c\n", k, 'A' + ships[k].y, '1' + ships[k].x);
#endif

	while(1) {
		print_field();
		if (hits == SHIPS)
			break;
		read_target(&x, &y);
		scan(x, y);
	}

	return 0;
}