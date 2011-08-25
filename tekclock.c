/* $Id$ */
/*
 * Copyright (c) 2010 Dimitri Sokolyuk <demon@dim13.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include <err.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tekplot.h"

#define	MAXX 4096
#define MAXY 3120

int	dflag = 0;

void
settimer(int sec, int usec)
{
	struct	itimerval itv;

	itv.it_value.tv_sec = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;

	setitimer(ITIMER_REAL, &itv, NULL);
}

void
usage(void)
{
	extern	char *__progname;

	(void)fprintf(stderr, "usage: %s [-d msec] [ILDA]\n", __progname);

	exit(1);
}

void
catch(int signo)
{
	if (signo != SIGALRM)
		dflag = 1;
	return;
}

#define	R	((MAXY) / 2 - 1)
#define	off	((MAXX - MAXY) / 2 - 1)

int
dot(int k, int r, int *x, int *y)
{
	double	phy;

	phy = M_PI * (90 - k) / 180;
	*x = r * cos(phy) + R + off;
	*y = r * sin(phy) + R;

	return 0;
}

int
drawdisk(void)
{
	int	i, x, y;

	for (i = 0; i <= 60; i++) {
		dot(6 * i, R, &x, &y);
		iplot(!!i, x, y);
	}

	return 0;
}

int
drawmarks(void)
{
	int	i, x, y, u, v;

	for (i = 0; i < 60; i++) {
		dot(i * 6, R, &x, &y);
		dot(i * 6, R - 50 * ((i % 5) ? 1 : 3), &u, &v);

		iplot(0, (int)x, (int)y);
		iplot(1, (int)u, (int)v);
	}
	return 0;
}


int
drawhands(struct tm *tm)
{
	int	ox, oy;
	int	hx, hy;
	int	mx, my;
	int	sx, sy;
	int	h, m, s;

	h = 30 * (tm->tm_hour % 12) + tm->tm_min / 2;
	m = 6 * tm->tm_min + tm->tm_sec / 10;
	s = 6 * tm->tm_sec;

	dot(0, 0, &ox, &oy);

	dot(s, R - 200, &sx, &sy);
	iplot(0, ox, oy);
	iplot(1, sx, sy);

	dot(m, R - 400, &mx, &my);
	iplot(0, ox, oy);
	iplot(1, mx, my);

	dot(h, R - 800, &hx, &hy);
	iplot(0, ox, oy);
	iplot(1, hx, hy);

	return 0;
}

int
main(int argc, char **argv)
{
	struct	sigaction sa;
	int	delay = 1;
	int	udelay = 0;
	time_t	t;
	struct	tm *tm;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = catch;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);

	settimer(delay, udelay);

	inittek();
	while (!dflag) {
		time(&t);
		tm = localtime(&t);

		page();

		drawdisk();
		drawmarks();
		drawhands(tm);

		fflush(stdout);

		sigsuspend(&sa.sa_mask);
	}
	endtek();

	return 0;
}
