// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include <assert.h>

void animupdate(Anim *a)
{
	a->d--;
	if(a->d > 0)
		return;
	a->d = a->delay;
	a->f++;
	if(a->f == a->len)
		a->f = 0;
}

void animdraw(Gfx *g, Anim *a, Point p)
{
	double y = a->row * a->h;
	double x = a->f * a->w;
	Rect clip = {
		{ x, y },
		{ x + a->w, y + a->h }
	};
	imgdrawreg(g, a->sheet, clip, p);
}

void animreset(Anim *a)
{
	a->f = 0;
	a->d = a->delay;
}
