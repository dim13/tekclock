/* $Id$ */

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "tekplot.h"

#define EXTRABITS	0x0f
#define FIVEBITS	0x1f
#define HIBITS		(FIVEBITS << SHIFTHI)
#define LOBITS		(FIVEBITS << SHIFTLO)
#define SHIFTHI		7
#define SHIFTLO		2
#define TWOBITS		0x03

#define ETX 3			/* VT page */
#define ENQ 5
#define BELL 7
#define BS 010			/* dec char spacing */
#define HT 011			/* inc char spacing */
#define LF 012			/* dec line spacing */
#define VT 013			/* inc line spacing */
#define FF 014			/* TEX page */
#define CR 015			/* Alpha mode */
#define SO 016
#define SI 017
#define SUB 032			/* Graphics-in mode */
#define ESC 033
#define FS 034			/* Point mode */
#define GS 035			/* Graphic mode */
#define RS 036			/* IPlot mode */
#define US 037			/* Alpha mode */
#define DEL 0177		/* more esc */

#define TEXT US
#define PEN GS
#define DOT FS
#define GIN SUB
#define PAGE FF
#define VTPAGE 3

enum { ALPHA, VECTOR, POINT, SPOINT, SPOINTFETCH, IPLOT, CROSSHAIR } plotf;

#define psleep(t)	usleep(1000 * (t))	/* Sleep for t milliseconds */

struct disps {
	int     scrnx[2];
	int     scrny[2];
}       dispxy;

int     ohy, oly, ohx, oex;
int     lastx, lasty;

int     side = 0;
int     teklf = 90;
int     Mechanical = 0;

int     Notatty;		/* Output is not a tty - likely pipe to phys */
int     waitflash;
int     XTerm;			/* running under XTerm */

static int pen, maxx, oldx, oldy;
#if 0
static char Zxystring[] = "4507430165672321";
#endif
static char xystring[] = "BJAIBFAEHJHIDFDE";


static const char pendown[] = "P";
static const char penup[] = " ";

void
esc(char c)
{
	putchar(ESC);
	putchar(c);
	fflush(stdout);
}

void
xycvt(int x, int y)
{
	int     c;
	char   *p;
	char    pbuf[8];
	p = pbuf;
	*p++ = 0;

	/* queue low order x */
	*p++ = ((x >> SHIFTLO) & FIVEBITS) | 0x40;

	/* if hi x changed, queue it and force xmsn of low order y */
	if (ohx != (c = ((x >> SHIFTHI) & FIVEBITS) | 0x20))
		*p++ = oly = ohx = c;

	/* calculate extra byte */
	c = (x & TWOBITS) | ((y & TWOBITS) << SHIFTLO) | 0x60;
	if (y & 0x1000)
		c |= 020;

	/* queue lo y and extra if extra changed */
	if (oex != c) {
		*p++ = oly = ((y >> SHIFTLO) & FIVEBITS) | 0x60;
		*p++ = oex = c;
	} else if (oly != (c = ((y >> SHIFTLO) & FIVEBITS) | 0x60))
		*p++ = oly = c;

	if (ohy != (c = ((y >> SHIFTHI) & FIVEBITS) | 0x20))
		*p++ = ohy = c;

	while ((c = *--p))
		putchar(c);

	if (waitflash) {
		if (((abs(x - lastx) > 1500) || (abs(y - lasty) > 1500)) && (plotf == VECTOR))
			ohy = oly = ohx = oex = 0;
		lastx = x;
		lasty = y;
	}
}

/*
 * i = 0  dark vector
 * i = -1  point plot
 * i = -32 to -126   special point plot z axis setting
 */

void
iplot(int i, int ix, int iy)
{

	if (Mechanical) {
		xyplot(i, ix, iy);
		return;
	}
	if (iy < 0 || iy > 4095 || ix < 0 || ix > 4095)
		return;		/* crude clip */

	if (i <= 0) {
		switch (i) {
		case -1:
			if (plotf != POINT) {
				putchar(DOT);
				plotf = POINT;
			}
			break;
		case 0:
			if (plotf != VECTOR && plotf != ALPHA)
				putchar(TEXT);
			putchar(PEN);
			plotf = VECTOR;
			break;
		default:
			if (plotf != SPOINT) {
				if (plotf != ALPHA)
					putchar(TEXT);

				putchar(ESC);
				putchar(DOT);

				plotf = SPOINT;
			}
			i = -i;
			if (i > 125)
				i = 125;
			if (i < 32)
				i = 32;
			putchar(i);
			break;
		}
	}
	xycvt(ix, iy);
	return;
}

void
alpha(void)
{
	if (Mechanical)
		printf(penup);

	if (waitflash && plotf == VECTOR) {
		putchar(0);
		putchar(0);
		putchar(0);
		putchar(0);
	}
	plotf = ALPHA;
	pen = 1;
	lastx = -1;
	lasty = -1;
	putchar(TEXT);
	fflush(stdout);
}

void
inittek(void)
{
	char   *p;
	int     t;
	static int done = 0;
	if (done)
		return;

	p = getenv("TERM");

	if (!p)
		return;

	if ((strncmp(p, "xterm", 5) == 0) || (strcmp(p, "dumb") == 0)) {
		teklf = 88;
		XTerm = 1;
		waitflash = 0;

		printf("%c[?38h", ESC);

	} else if (strcmp(p, "tek4014") == 0)
		teklf = 88;
	else {
		waitflash = isdigit(*p);
		t = atoi(p);
		if ((t > 4012) && (t < 4020))
			teklf = 56;
		else
			teklf = 88;
	}

	done = 1;
}

void
endtek(void)
{
	if (XTerm) {
		putchar(TEXT);
		esc(VTPAGE);
	}
}

void
page(void)
{
	inittek();

	fflush(stderr);
	fflush(stdout);

	if (!isatty(1))
		psleep(200);

	dispxy.scrnx[0] = 0;
	dispxy.scrnx[1] = 2048;
	dispxy.scrny[0] = 3070;
	dispxy.scrny[1] = 3070;

	ohy = 0;
	oly = 0;
	ohx = 0;
	oex = 0;

	lastx = -1;
	lasty = -1;

	esc(PAGE);
	plotf = ALPHA;

	if (waitflash)
		psleep(1500);
}

/* VARARGS */
void
disp(char *fmt,...)
{
	va_list ap;
	alpha();

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	fflush(stdout);
}

/* VARARGS */
void
spad(char *fmt,...)
{
	va_list ap;
	inittek();

	if ((dispxy.scrny[side] -= teklf) < 0)
		dispxy.scrny[side] = 0;

	iplot(0, dispxy.scrnx[side], dispxy.scrny[side]);

	alpha();

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	fflush(stdout);
	psleep(40);
}

void
dispii(char *s, int *t)
{
	printf("%s ", s);
	fflush(stdout);
	scanf("%d", t);
}

void
spadii(char *s, int *t)
{
	char    line[80];
	spad(s);
	fgets(line, 77, stdin);
	if (line[0])
		sscanf(line, "%d", t);
}

void
spadiid(char *s, int *t)
{
	char    line[80];
	spad(s);
	printf(" [%d]: ", *t);
	fflush(stdout);
	fgets(line, 77, stdin);
	if (line[0])
		sscanf(line, "%d", t);
}

void
xyplot(int i, int ix, int iy)
{

	if (i == 0) {
		if (pen)
			printf(penup);
		pen = 0;
	} else if (i < 0) {
		if (pen)
			printf(penup);
		xymove(ix, iy);
		printf(pendown);
		printf("AHBD");
		pen = 1;
		return;
	} else if (pen == 0) {
		printf(pendown);
		pen++;
	}
	xymove(ix, iy);
}

void
xymove(int x, int y)
{
	int     ix, iy, sindex, idelta;
	int     ch1, ch2;
	if (Mechanical < 0) {
		iy = -x;
		x = y;
		y = iy;
	}
	sindex = 0;
	if (x > maxx)
		maxx = x;

	if ((ix = (oldx - x)) < 0) {
		sindex++;
		ix = -ix;
	}
	if ((iy = (oldy - y)) < 0) {
		sindex += 2;
		iy = -iy;
	}
	if (iy > ix)
		sindex += 4;
	else if (ix > iy) {
		idelta = ix;
		ix = iy;
		iy = idelta;
	}
	if (iy == 0)
		return;

	sindex <<= 1;

	ch1 = xystring[sindex];
	ch2 = xystring[sindex + 1];

#define iless ix
#define igreat iy
#define icount sindex
	icount = iy;

	iless <<= 1;
	idelta = iless - igreat;
	igreat <<= 1;

	while (--icount >= 0) {
		if (idelta >= 0) {
			putchar(ch2);
			idelta -= igreat;
		} else {
			putchar(ch1);
		}
		idelta += iless;
	}
	oldx = x;
	oldy = y;
}

/*
 * mech(n)
 * 0:	vectors
 * >0:	incremental plotting
 * <0:	incremental plotting rotated 90 degrees clockwise
 * 	on a drum plotter this makes y axis the long one
 */

void
mech(int n)
{
	pen = 1;
	alpha();
	iplot(-1, 0, 0);
	alpha();

	if (n)
		putchar(RS);

	printf(penup);
	Mechanical = n;
}

/*
 * xyres()
 * tells xymove that the pen is at 0,0
 */

void
xyres(void)
{
	oldx = 0;
	oldy = 0;
	maxx = 0;
}

/*
 * mpage(idelta)
 * performs a move to the coordinate maxx+idelta,0
 * 	where maxx is the largest positive x excursion of the
 * 	plotter since the last call to mpage.
 */

void
mpage(int idelta)
{
	iplot(0, maxx + idelta, 0);
	xyres();
}

void
setchsize(int n)
{				/* set character size */
	switch (n) {
	case 4:
		esc(';');
		break;
	case 3:
		esc(':');
		break;
	case 2:
		esc('9');
		break;
	default:
		esc('8');
		break;
	}
}

void
setzw(int n)
{				/* set beam and vector mode */
	esc(n);
}
