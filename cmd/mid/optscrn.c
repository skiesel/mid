// Copyright © 2011 Steve McCoy and Ethan Burns
// Licensed under the MIT License. See LICENSE for details.
#include "../../include/mid.h"
#include "../../include/log.h"
#include "game.h"
#include <string.h>
#include <ctype.h>

typedef struct Opts Opts;
struct Opts{
	char kmap[Nactions];
	Rect hilit[Nactions];
	Rect volhilit;
	_Bool volset;
	int curkey;
	Rect okay;
	_Bool isokay;
	Rect cancel;
	_Bool iscancel;
	Txt *txt;
	Sfx *testsfx;
};

enum{
	Pad = TxtSzMedium/2,
};

static void update(Scrn *s, Scrnstk *stk);
static void draw(Scrn *s, Gfx *g);
static void handle(Scrn *s, Scrnstk *stk, Event *e);
static void optsfree(Scrn *s);

static Scrnmt optsmt = {
	update,
	draw,
	handle,
	optsfree
};

Scrn *optscrnnew(void){
	static Opts opts = {};
	static Scrn s = {};

	memcpy(opts.kmap, kmap, Nactions);
	opts.curkey = Mvleft;
	Txtinfo ti = { TxtSzMedium };
	opts.txt = resrcacq(txt, TxtStyleMenu, &ti);
	if(!opts.txt)
		return NULL;

	opts.testsfx = resrcacq(sfx, "sfx/ow.wav", 0);
	if(!opts.testsfx)
		return NULL;

	Point loc = { Pad, Pad };
	for(int i = Mvleft; i < Nactions; i++){
		opts.hilit[i] = (Rect){
			loc,
			vecadd(loc, (Point){ TxtSzMedium*5, TxtSzMedium })
		};
		loc.y = opts.hilit[i].b.y + Pad;
	}

	loc.y += Pad*2;
	opts.volhilit = (Rect){
		loc,
		vecadd(loc, (Point){ TxtSzMedium*7, TxtSzMedium })
	};

	loc.y = opts.volhilit.b.y + Pad*3;
	opts.okay = (Rect){
		loc,
		vecadd(loc, (Point){ TxtSzMedium*4, TxtSzMedium })
	};

	loc.x = opts.okay.b.x + Pad;
	opts.cancel = (Rect){
		loc,
		vecadd(loc, (Point){ TxtSzMedium*4, TxtSzMedium })
	};

	s.mt = &optsmt;
	s.data = &opts;
	return &s;
}

static void update(Scrn *s, Scrnstk *stk){
	// nothing
}

static void draw(Scrn *s, Gfx *g){
	static char *names[] = {
		[Mvleft] = "Left",
		[Mvright] = "Right",
		[Mvact] = "Action",
		[Mvjump] = "Jump",
		[Mvinv] = "Menu",
		[Mvsword] = "Sword",
	};

	Opts *opt = s->data;

	gfxclear(g, (Color){ 240, 240, 240 });

	Color hilit = { 255, 219, 0 };

	for(int i = Mvleft; i < Nactions; i++){
		if(i == opt->curkey){
			Rect h = {
				vecadd(opt->hilit[i].a, (Point){-4,-4}),
				vecadd(opt->hilit[i].b, (Point){4,4})
			};
			gfxfillrect(g, h, hilit);
		}
		txtdraw(g, opt->txt, opt->hilit[i].a, "%s", names[i]);
		Point cd = txtdims(opt->txt, "%c", opt->kmap[i]);
		cd.x = -cd.x;
		cd.y = 0;
		Point cp = vecadd(opt->hilit[i].b, cd);
		cp.y = opt->hilit[i].a.y;
		txtdraw(g, opt->txt, cp, "%c", opt->kmap[i]);
	}

	if(opt->volset){
		Rect h = {
			vecadd(opt->volhilit.a, (Point){-4,-4}),
			vecadd(opt->volhilit.b, (Point){4,4})
		};
		gfxfillrect(g, h, hilit);
	}
	txtdraw(g, opt->txt, opt->volhilit.a, "Volume:");
	float vpcnt = sndvol(-1) / (float)SndVolMax * 100;
	Point pd = txtdims(opt->txt, "%.1f%%", vpcnt);
	pd.x = -pd.x;
	pd.y = 0;
	Point pp = vecadd(opt->volhilit.b, pd);
	pp.y = opt->volhilit.a.y;
	txtdraw(g, opt->txt, pp, "%.1f%%", vpcnt);

	Color butt = { 200, 200, 200 };
	Rect oh = {
		vecadd(opt->okay.a, (Point){-4,-4}),
		vecadd(opt->okay.b, (Point){4,4})
	};
	gfxfillrect(g, oh, butt);
	Point op = {
		opt->okay.a.x + (opt->okay.b.x - opt->okay.a.x)/2 - txtdims(opt->txt, "OK").x/2,
		opt->okay.a.y
	};
	txtdraw(g, opt->txt, op, "OK");
	Rect ch = {
		vecadd(opt->cancel.a, (Point){-4,-4}),
		vecadd(opt->cancel.b, (Point){4,4})
	};
	gfxfillrect(g, ch, butt);
	Point cp = {
		opt->cancel.a.x + (opt->cancel.b.x - opt->cancel.a.x)/2 - txtdims(opt->txt, "Cancel").x/2,
		opt->cancel.a.y
	};
	txtdraw(g, opt->txt, cp, "Cancel");

	gfxflip(g);
}

static void handle(Scrn *s, Scrnstk *stk, Event *e){
	Opts *opt = s->data;

	if(e->type == Mousebt && e->down){
		Point m = { e->x, e->y };

		for(int i = Mvleft; i < Nactions; i++)
			if(rectcontains(opt->hilit[i], m)){
				opt->curkey = i;
				opt->volset = 0;
				return;
			}

		if(rectcontains(opt->volhilit, m)){
			opt->volset = 1;
			opt->curkey = Nactions;
			return;
		}

		if(rectcontains(opt->okay, m)){
			opt->isokay = 1;
			scrnstkpop(stk);
			return;
		}

		if(rectcontains(opt->cancel, m)){
			opt->iscancel = 1;
			scrnstkpop(stk);
			return;
		}

		return;
	}

	if(e->type != Keychng || !e->down)
		return;

	if(!isalpha(e->key))
		return;

	if(opt->volset){
		int curvol = sndvol(-1);
		if(e->key == opt->kmap[Mvleft] && curvol > 0)
			sndvol(curvol-1);
		else if(e->key == opt->kmap[Mvright] && curvol < SndVolMax)
			sndvol(curvol+1);
		sfxplay(opt->testsfx);
		return;
	}

	if(e->repeat)
		return;

	if(opt->curkey == Nactions){
		opt->curkey = Mvleft;
		return;
	}

	int ck = opt->kmap[opt->curkey];
	for(int i = Mvleft; i < Nactions; i++)
		if(opt->kmap[i] == e->key){
			opt->kmap[i] = ck;
			break;
		}
	opt->kmap[opt->curkey] = e->key;
	opt->curkey++;
}

static void optsfree(Scrn *s){
	Opts *opt = s->data;

	if(opt->isokay){
		memcpy(kmap, opt->kmap, Nactions);
		//TODO: save to a file in appdata, also load at game start iff exists
	}

	if(opt->iscancel){
		sndvol(SndVolDefault);
	}

	memset(opt, 0, sizeof(*opt));
}