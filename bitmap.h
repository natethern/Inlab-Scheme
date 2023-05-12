/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: bitmap.h,v 4.102 2006/04/29 09:13:22 tommy Exp $
 */

typedef struct { 
  int orow;
  int ocol;
  int h;
  int w;
  float xres;
  float yres;
  int rowbytes;
  char* data;
} BITMAP;

typedef struct { 
  int orow;
  int ocol;
  int h;
  int w;
  float xres;
  float yres;
  unsigned char* data;
} GREYMAP;

typedef struct {
  int orow;
  int ocol;
  int h;
  int w;
  float xres;
  float yres;
  unsigned char* data;
} COLORMAP;

BITMAP*	bitmap_readtiff(char* filename, int directory);
BITMAP*	bitmap_readpng(char* filename);
BITMAP*	bitmap_readxbm(char* filename);
BITMAP*	bitmap_dummy();
BITMAP*	bitmap_create(int, int);
BITMAP*	bitmap_vccreate(int, int);
BITMAP*	bitmap_extract(BITMAP*, int, int);
BITMAP*	bitmap_copy(BITMAP*);
BITMAP*	bitmap_rowdiff(BITMAP*);
BITMAP*	bitmap_rowdiff_reverse(BITMAP*);
BITMAP*	bitmap_scale(BITMAP*,double,double);
BITMAP*	bitmap_scaleabsolute(BITMAP*,int,int);
BITMAP*	bitmap_rrotate(BITMAP*);
BITMAP*	bitmap_lrotate(BITMAP*);
BITMAP*	bitmap_half(BITMAP*);
BITMAP*	bitmap_8(BITMAP*);
BITMAP*	bitmap_half4(BITMAP*);
BITMAP*	bitmap_double(BITMAP*);
BITMAP*	bitmap_vcdouble(BITMAP*);
BITMAP*	greymap_to_bitmap(GREYMAP*, int);
BITMAP*	bitmap_crop(BITMAP*, int, int, int, int);

void bitmap_destroy(BITMAP* bitmap);

GREYMAP*	greymap_scale(GREYMAP*,double,double);
GREYMAP*	greymap_scaleabsolute(GREYMAP*,int,int);
GREYMAP*	greymap_create(int, int);
GREYMAP*	greymap_readpng(char*);
GREYMAP*	bitmap_to_greymap(BITMAP*);
GREYMAP*	greymap_copy(GREYMAP*);
GREYMAP*	greymap_crop(GREYMAP*, int, int, int, int);
GREYMAP*	greymap_smooth_region(GREYMAP*, int, int, int, int, int);

char*		patimg_to_tiff(unsigned char*, unsigned char*);
char*		st33_to_tiff(int, unsigned char*, unsigned char*);

COLORMAP*	colormap_create(int, int);




