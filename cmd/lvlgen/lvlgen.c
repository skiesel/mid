// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/times.h>
#include <assert.h>
#include "../../include/mid.h"
#include "../../include/log.h"
#include "../../include/rng.h"
#include "lvlgen.h"

static int rng(Rng *, int, char *[]);
static void init(Lvl *l);
extern void lvlwrite(FILE *, Lvl *);
/* Fill in a z-layer and return a Loc for a good door placement to the
 * next zlayer. */
static Loc zlayer(Loc loc, Lvl *lvl);
static void buildpath(Lvl *lvl, Path *p, Loc loc);

Rng r;

int main(int argc, char *argv[])
{
	loginit(NULL);

	if (argc != 4 && argc != 6)
		return 1;

	int usdargs = rng(&r, argc, argv);
	argc -= usdargs;
	argv += usdargs;

	int w = strtol(argv[1], NULL, 10);
	int h = strtol(argv[2], NULL, 10);
	int d = strtol(argv[3], NULL, 10);
	Lvl *lvl = lvlnew(d, w, h);

	mvini();
	init(lvl);

	Loc loc = (Loc) { 2, 2 };
	blk(lvl, loc.x, loc.y, 0)->tile = ' ';
	for (int z = 0; z < d; z++) {
		loc = zlayer(loc, lvl);
		if (z < d - 1) {
			blk(lvl, loc.x, loc.y, lvl->z)->tile = '>';
			water(lvl);
			lvl->z++;
			blk(lvl, loc.x, loc.y, lvl->z)->tile = '<';
		}
	}

	extradoors(&r, lvl);

	lvlwrite(stdout, lvl);
	lvlfree(lvl);

	return 0;
}

static int rng(Rng *r, int argc, char *argv[])
{
	int args = 0;
	clock_t seed = 0;

	if (argv[1][0] == '-' && argv[1][1] == 's') {
		seed = strtol(argv[2], NULL, 10);
		args = 2;
	} else {
		struct tms tm;
		seed = times(&tm);
		pr("itmgen seed = %lu", (unsigned long) seed);
	}
	rnginit(r, seed);

	return args;
}

static void init(Lvl *l)
{
	for (int z = 0; z < l->d; z++) {
	for (int y = 0; y < l->h; y++) {
	for (int x = 0; x < l->w; x++) {
		int c = '.';
		if (x == 0 || x == l->w - 1 || y == 0 || y == l->h - 1)
				c = '#';
			blk(l, x, y, z)->tile = c;
	}
	}
	}
}

static Loc zlayer(Loc loc, Lvl *lvl)
{
	Path *p = pathnew(lvl);
	buildpath(lvl, p, loc);
	Loc nxt = doorloc(lvl, p, loc);
	pathfree(p);
	morereach(lvl);
	return nxt;
}

enum { Minbr = 3, Maxbr = 9 };

static void buildpath(Lvl *lvl, Path *p, Loc loc)
{
	unsigned int br = rnd(Minbr, Maxbr);
	for (int i = 0; i < br; i++) {
		unsigned int base = rnd(0, Nmoves);
		for (int j = 0; j < Nmoves; j++) {
			unsigned int mv = (base + j) % Nmoves;
			Seg s = segmk(loc, &moves[mv]);
			if (pathadd(lvl, p, s)) {
				buildpath(lvl, p, s.l1);
				break;
			}
		}
	}
}

unsigned int rnd(int min, int max)
{
	return rngintincl(&r, min, max);
}
