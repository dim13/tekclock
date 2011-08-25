/* $Id$ */

#ifndef _TEKPLOT_H
#define _TEKPLOT_H

/* arguments to setzw() */
#define TEKNZNV	96		/* Normal Z axis and Normal vectors */
#define TEKNZDOT 97		/* Normal Z dotted vectors */
#define TEKNZDOTDASH 98		/* Normal Z dot-dash vectors */
#define TEKNZSHORTDASH 99	/* short dash */
#define TEKNZLONGDASH 100	/* long dash */
#define TEKWZNV 112		/* Write-thru,  Normal vectors */
#define TEKWZDOT 113		/* Write-thru,  Dotted vectors */
#define TEKDZNV 108		/* Defocused,  Normal vectors */

__BEGIN_DECLS

void    iplot(int, int, int);	/* i x y */
void    alpha(void);
void    page(void);
void    setzw(int);
int     kurse(int *, int *, int *);	/* char x y */

void    esc(char);
void    xycvt(int, int);
void    alpha(void);
void    inittek(void);
void    endtek(void);
void    page(void);
void    disp(char *,...);
void    spad(char *,...);
void    dispii(char *, int *);
void    spadii(char *, int *);
void    spadiid(char *, int *);
void    xyplot(int, int, int);
void    xymove(int, int);
void    mech(int);
void    xyres(void);
void    mpage(int);
void    setchsize(int);
void    setzw(int);

__END_DECLS

#endif
