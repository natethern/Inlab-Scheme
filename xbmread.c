/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: xbmread.c,v 4.102 2006/04/29 09:13:22 tommy Exp $
 *
 * Based on code derived from xlib:
 *
 * Copyright (c) 1987  X Consortium
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the name of the X Consortium shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization
 * from the X Consortium.
 * 
 */

/*
 *	Code to read bitmaps from disk files. Interprets 
 *	data from X10 and X11 bitmap files and creates
 *	Pixmap representations of files. Returns Pixmap
 *	ID and specifics about image.
 *
 *	Modified for speedup by Jim Becker, changed image
 *	data parsing logic (removed some fscanf()s). 
 *	Aug 5, 1988
 *
 * Note that this file and ../Xmu/RdBitF.c look very similar....  Keep them
 * that way (but don't use common source code so that people can have one 
 * without the other).
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <scheme.h>

#define MAX_SIZE 255
#define False 0
#define True 1

typedef int Bool;


/* shared data for the image read/parse logic */

static short hexTable[256];		/* conversion value */
static Bool initialized = False;	/* easier to fill in at run time */

static short Ctable[256];

/*
 *	Table index for the hex values. Initialized once, first time.
 *	Used for translation value or delimiter significance lookup.
 */

static void initHexTable()
{
  int i;
    /*
     * We build the table at run time for several reasons:
     *
     *     1.  portable to non-ASCII machines.
     *     2.  still reentrant since we set the init flag after setting table.
     *     3.  easier to extend.
     *     4.  less prone to bugs.
     */
    hexTable['0'] = 0;	hexTable['1'] = 1;
    hexTable['2'] = 2;	hexTable['3'] = 3;
    hexTable['4'] = 4;	hexTable['5'] = 5;
    hexTable['6'] = 6;	hexTable['7'] = 7;
    hexTable['8'] = 8;	hexTable['9'] = 9;
    hexTable['A'] = 10;	hexTable['B'] = 11;
    hexTable['C'] = 12;	hexTable['D'] = 13;
    hexTable['E'] = 14;	hexTable['F'] = 15;
    hexTable['a'] = 10;	hexTable['b'] = 11;
    hexTable['c'] = 12;	hexTable['d'] = 13;
    hexTable['e'] = 14;	hexTable['f'] = 15;

    /* delimiters of significance are flagged w/ negative value */
    hexTable[' '] = -1;	hexTable[','] = -1;
    hexTable['}'] = -1;	hexTable['\n'] = -1;
    hexTable['\t'] = -1;

    for(i=0; i<256; i++) {
      Ctable[i]=((i&0x80)>>7)|((i&0x40)>>5)|((i&0x20)>>3)|((i&0x10)>>1)|
		((i&0x08)<<1)|((i&0x04)<<3)|((i&0x02)<<5)|((i&0x01)<<7);
    }
	
    initialized = True;
}

/*
 *	read next hex value in the input stream, return -1 if EOF
 */

static NextInt (fstream)
    FILE *fstream;
{
    int	ch;
    int	value = 0;
    int gotone = 0;
    int done = 0;
    
    /* loop, accumulate hex value until find delimiter  */
    /* skip any initial delimiters found in read stream */

    while (!done) {
	ch = getc(fstream);
	if (ch == EOF) {
	    value	= -1;
	    done++;
	} else {
	    /* trim high bits, check type and accumulate */
	    ch &= 0xff;
	    if (isascii(ch) && isxdigit(ch)) {
		value = (value << 4) + hexTable[ch];
		gotone++;
	    } else if ((hexTable[ch]) < 0 && gotone)
	      done++;
	}
    }
    if(value<0)
      return(value);
    else 
      return Ctable[value];
}

int SchemeReadXbm(
    char *filename,
    unsigned int *width,                /* RETURNED */
    unsigned int *height,               /* RETURNED */
    unsigned char **data,               /* RETURNED */
    int *x_hot,                         /* RETURNED */
    int *y_hot)                         /* RETURNED */
{
    FILE *fstream;			/* handle on file  */
    unsigned char *bits = NULL;		/* working variable */
    char line[MAX_SIZE];		/* input line from file */
    int size;				/* number of bytes of data */
    char name_and_type[MAX_SIZE];	/* an input line */
    char *type;				/* for parsing */
    int value;				/* from an input line */
    int version10p;			/* boolean, old format */
    int padding;			/* to handle alignment */
    int bytes_per_line;			/* per scanline of data */
    unsigned int ww = 0;		/* width */
    unsigned int hh = 0;		/* height */
    int hx = -1;			/* x hotspot */
    int hy = -1;			/* y hotspot */

    /* first time initialization */
    if (initialized == False) initHexTable();

    if (!(fstream = fopen(filename, "r")))
	return SCHEME_ERROR;

    /* error cleanup and return macro	*/

/*
#define	RETURN(code) \
{ if (bits) SchemeFree ((char *)bits); fclose (fstream); return code; }
*/

#define	RETURN(code) \
{ fclose (fstream); return code; }

    while (fgets(line, MAX_SIZE, fstream)) {
	if (strlen(line) == MAX_SIZE-1)
	    RETURN (SCHEME_ERROR);
	if (sscanf(line,"#define %s %d",name_and_type,&value) == 2) {
	    if (!(type = strrchr(name_and_type, '_')))
	      type = name_and_type;
	    else
	      type++;

	    if (!strcmp("width", type))
	      ww = (unsigned int) value;
	    if (!strcmp("height", type))
	      hh = (unsigned int) value;
	    if (!strcmp("hot", type)) {
		if (type-- == name_and_type || type-- == name_and_type)
		  continue;
		if (!strcmp("x_hot", type))
		  hx = value;
		if (!strcmp("y_hot", type))
		  hy = value;
	    }
	    continue;
	}
    
	if (sscanf(line, "static short %s = {", name_and_type) == 1)
	  version10p = 1;
	else if (sscanf(line,"static unsigned char %s = {",name_and_type) == 1)
	  version10p = 0;
	else if (sscanf(line, "static char %s = {", name_and_type) == 1)
	  version10p = 0;
	else
	  continue;

	if (!(type = strrchr(name_and_type, '_')))
	  type = name_and_type;
	else
	  type++;

	if (strcmp("bits[]", type))
	  continue;
    
	if (!ww || !hh)
	  RETURN (SCHEME_ERROR);

	if ((ww % 16) && ((ww % 16) < 9) && version10p)
	  padding = 1;
	else
	  padding = 0;

	bytes_per_line = (ww+7)/8 + padding;

	size = bytes_per_line * hh;
	bits = (unsigned char *) SchemeMalloc ((unsigned int) size);
	if (!bits) 
	  RETURN (SCHEME_ERROR);

	if (version10p) {
	    unsigned char *ptr;
	    int bytes;

	    for (bytes=0, ptr=bits; bytes<size; (bytes += 2)) {
		if ((value = NextInt(fstream)) < 0)
		  RETURN (SCHEME_ERROR);
		*(ptr++) = value;
		if (!padding || ((bytes+2) % bytes_per_line))
		  *(ptr++) = value >> 8;
	    }
	} else {
	    unsigned char *ptr;
	    int bytes;

	    for (bytes=0, ptr=bits; bytes<size; bytes++, ptr++) {
		if ((value = NextInt(fstream)) < 0) 
		  RETURN (SCHEME_ERROR);
		*ptr=value;
	    }
	}
    }					/* end while */

    /*
    fclose(fstream);
    */
    if (!bits)
	RETURN (SCHEME_ERROR);

    *data = bits;
    *width = ww;
    *height = hh;
    if (x_hot) *x_hot = hx;
    if (y_hot) *y_hot = hy;

    RETURN (SCHEME_OK);
}

