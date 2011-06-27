#include "../../include/mid.h"

struct Enemymt{
	void (*free)(Enemy*);
	void (*update)(Enemy*, Player*, Lvl*);
	void (*draw)(Enemy*, Gfx*, Point tr);
};

static _Bool untiinit(Enemy *, int, int);
static void untifree(Enemy*);
static void untiupdate(Enemy*,Player*,Lvl*);
static void untidraw(Enemy*,Gfx*,Point);
static Enemymt untimt = {
	untifree,
	untiupdate,
	untidraw,
};

typedef struct Unti Unti;
struct Unti{
	Color c;
	Img *img;
};

static _Bool (*spawns[])(Enemy*, int, int) = {
	['u'] = untiinit,
};

_Bool enemyinit(Enemy *e, unsigned char id, int x, int y){
	if(id >= sizeof(spawns)/sizeof(spawns[0]))
		return 0;

	return spawns[id](e, x, y);
}

void enemyfree(Enemy *e){
	if(!e->mt)
		return;
	e->mt->free(e);
	e->mt = 0;
	e->hp = 0;
}

void enemyupdate(Enemy *e, Player *p, Lvl *l){
	if(e->mt) e->mt->update(e, p, l);
}

void enemydraw(Enemy *e, Gfx *g, Point tr){
	if(e->mt) e->mt->draw(e, g, tr);
}

static _Bool untiinit(Enemy *e, int x, int y){
	bodyinit(&e->b, x * Twidth, y * Theight);
	e->hp = 1;

	Unti *u = xalloc(1, sizeof(*u));
	u->c = (Color){ 255, 55, 55, 255 };
	u->img = resrcacq(imgs, "img/unti.png", 0);

	e->mt = &untimt;
	e->data = u;
	return 1;
}

static void untifree(Enemy *e){
	resrcrel(imgs, "img/unti.png", 0);
	xfree(e->data);
}

static void untiupdate(Enemy *e, Player *p, Lvl *l){
	// Real enemies will do AI
	Unti *u = e->data;

	bodyupdate(&e->b, l);

	if(isect(e->b.bbox, playerbox(p))){
		u->c.b = 255;
		playerdmg(p, 3);
	}else
		u->c.b = 55;

	if(isect(e->b.bbox, p->sw.loc[p->sw.cur]))
		e->hp--;

	u->c.r++;
}

static void untidraw(Enemy *e, Gfx *g, Point tr){
	Unti *u = e->data;

	Rect r = {
		{e->b.bbox.a.x + tr.x, e->b.bbox.a.y + tr.y},
		{e->b.bbox.b.x + tr.x, e->b.bbox.b.y + tr.y}
	};
	//gfxfillrect(g, r, u->c);
	imgdraw(g, u->img, (Point){r.a.x, r.a.y});
}
