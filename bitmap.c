/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: bitmap.c,v 4.102 2006/04/29 09:13:22 tommy Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bitmap.h>
#include <tiffio.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <scheme.h>
#include <assert.h>
#include <math.h>

#define GETPIXEL(b,r,c) ((*(((b)->data)+(((b)->rowbytes)*(r))+((c)/8)) & (0x80>>((c)%8)))?1:0)
#define SETPIXEL(b,r,c) *(((b)->data)+(((b)->rowbytes)*(r))+((c)/8)) |= (0x80>>((c)%8))
#define CLEARPIXEL(b,r,c) *(((b)->data)+(((b)->rowbytes)*(r))+((c)/8)) &= (~(0x80>>((c)%8)))
#define GREYMAP_GETPIXEL(g,r,c) (*((g)->data+(r)*((g)->w)+(c)))

BITMAP* bitmap_create(int h, int w) {
  BITMAP* rc;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=-1;
    rc->ocol=-1;
    rc->xres=-1.0;
    rc->yres=-1.0;
    rc->h=h;
    rc->w=w;
    rc->rowbytes=(w+7)/8;
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->rowbytes);
    return(rc);
  }
}

BITMAP* bitmap_vccreate(int h, int w) {
  BITMAP* rc;
  int row,col;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=-1;
    rc->ocol=-1;
    rc->xres=-1.0;
    rc->yres=-1.0;
    rc->h=h;
    rc->w=w;
    rc->rowbytes=(w+7)/8;
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->rowbytes);
    for(row=0; row<rc->h; row++) {
      for(col=0; col<rc->w; col++) {
        if(rand() % 2) {
          SETPIXEL(rc,row,col);
        }
      }
    }
    return(rc);
  }
}

GREYMAP* greymap_create(int h, int w) {
  GREYMAP* rc;
  if ((rc = (GREYMAP*) SchemeMalloc(sizeof(GREYMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=-1;
    rc->ocol=-1;
    rc->xres=-1.0;
    rc->yres=-1.0;
    rc->h=h;
    rc->w=w;
    if((rc->data = (unsigned char*) SchemeMalloc(rc->h*rc->w)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->w);
    return(rc);
  }
}

COLORMAP* colormap_create(int h, int w) {
  COLORMAP* rc;
  if ((rc = (COLORMAP*) SchemeMalloc(sizeof(COLORMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=-1;
    rc->ocol=-1;
    rc->xres=-1.0;
    rc->yres=-1.0;
    rc->h=h;
    rc->w=w;
    if((rc->data = (unsigned char*) SchemeMalloc(rc->h*rc->w*4)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->w*4);
    return(rc);
  }
}

BITMAP* bitmap_copy(BITMAP* b) {
  BITMAP* rc;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    rc->h=b->h;
    rc->w=b->w;
    rc->xres=b->xres;
    rc->yres=b->yres;
    rc->rowbytes=b->rowbytes;
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    memcpy(rc->data, b->data, rc->rowbytes*rc->h);
    return(rc);
  }
}


GREYMAP* greymap_copy(GREYMAP* b) {
  GREYMAP* rc;
  if ((rc = (GREYMAP*) SchemeMalloc(sizeof(GREYMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    rc->h=b->h;
    rc->w=b->w;
    rc->xres=b->xres;
    rc->yres=b->yres;
    if((rc->data = (unsigned char*) SchemeMalloc(rc->h*rc->w)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    memcpy(rc->data, b->data, rc->w*rc->h);
    return(rc);
  }
}


BITMAP* bitmap_rowdiff(BITMAP* b) {
  BITMAP* rc;
  int r,c;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    rc->h=b->h;
    rc->w=b->w;
    rc->xres=b->xres;
    rc->yres=b->yres;
    rc->rowbytes=b->rowbytes;
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->rowbytes);
    for(r=0; r<rc->h; r++) {
      for(c=0; c<rc->w; c++) {
	if(r == 0) {
	  if(0 != GETPIXEL(b,r,c)) {
            SETPIXEL(rc,r,c);
	  }
	} else {
	  if(GETPIXEL(b,r-1,c) != GETPIXEL(b,r,c)) {
            SETPIXEL(rc,r,c);
	  }
	}
      }
    }
    return(rc);
  }
}

BITMAP* bitmap_rowdiff_reverse(BITMAP* b) {
  BITMAP* rc;
  int r,c;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    rc->h=b->h;
    rc->w=b->w;
    rc->xres=b->xres;
    rc->yres=b->yres;
    rc->rowbytes=b->rowbytes;
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->rowbytes);
    for(r=0; r<rc->h; r++) {
      for(c=0; c<rc->w; c++) {
	if(r == 0) {
	  if(GETPIXEL(b,r,c)) {
            SETPIXEL(rc,r,c);
	  } 
	} else {
	  if(GETPIXEL(b,r,c)) {
	    if(GETPIXEL(rc,r-1,c) == 0) {
	      SETPIXEL(rc,r,c);
	    }
	  } else {
	    if(GETPIXEL(rc,r-1,c)) {
	      SETPIXEL(rc,r,c);
	    }
	  }
	}
      }
    }
    return(rc);
  }
}

BITMAP* greymap_to_bitmap(GREYMAP* g, int val) {
  BITMAP* rc;
  int r,c;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=g->orow;
    rc->ocol=g->ocol;
    rc->h=g->h;
    rc->w=g->w;
    rc->xres=g->xres;
    rc->yres=g->yres;
    rc->rowbytes=(rc->w+7)/8;
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->rowbytes);
    for(r=0; r<rc->h; r++) {
      for(c=0; c<rc->w; c++) {
	if(GREYMAP_GETPIXEL(g,r,c) >= val) {
          SETPIXEL(rc,r,c);
	}
      }
    }
    return(rc);
  }
}

GREYMAP* bitmap_to_greymap(BITMAP* b) {
  GREYMAP* rc;
  int r,c;
  if ((rc = (GREYMAP*) SchemeMalloc(sizeof(GREYMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    rc->h=b->h;
    rc->w=b->w;
    rc->xres=b->xres;
    rc->yres=b->yres;
    if((rc->data = (unsigned char*) SchemeMalloc(rc->h*rc->w)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    for(r=0; r<rc->h; r++) {
      for(c=0; c<rc->w; c++) {
	if(GETPIXEL(b,r,c)) {
          (*((rc)->data+(r)*((rc)->w)+(c))) = 255;
	} else {
          (*((rc)->data+(r)*((rc)->w)+(c))) = 0;
	}
      }
    }
    return(rc);
  }
}

BITMAP* bitmap_readtiff(char* filename, int directory) {
  BITMAP* rc;
  int i;

  TIFF* 	tiff_file;
  int 		tfDirectory=0;
  uint16	tfBitsPerSample;
  uint16	tfSamplesPerPixel;  
  uint16	tfPlanarConfiguration;
  uint16	tfPhotometricInterpretation;
  uint16	tfGrayResponseUnit;
  uint32	tfImageDepth;
  uint32	tfImageWidth;
  uint32	tfImageHeight;
  float		tfXRES, tfYRES;
  int		tfBytesPerRow;

  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {

    /*
     * Erst einmal das TIFF-File aufmachen ...
     *
     */

    if((tiff_file = TIFFOpen(filename,"r")) == NULL) {
      fprintf(stderr,"WARNING: cannot open %s as TIFF.\n",filename);
      SchemeFree(rc);
      return(NULL);
    }

    /*
     * Dann der Directory-Seek ...
     *
     */

    if(directory != 0) {
      if(!TIFFSetDirectory(tiff_file,directory)) {
        /*
        fprintf(stderr,"WARNING: can't seek to directory in %s.\n",filename);
        */
        SchemeFree(rc);
        TIFFClose(tiff_file);
        return(NULL);
      }
    }

    /*
     * Breite und Hoehe holen ...
     *
     */

    TIFFGetField(tiff_file, TIFFTAG_IMAGEWIDTH, &tfImageWidth);

    /*
    fprintf(stderr,"DEBUG: %d Width, %d Height\n", tfImageWidth, tfImageHeight);
    */

    TIFFGetField(tiff_file, TIFFTAG_IMAGELENGTH, &tfImageHeight);

    /*
    fprintf(stderr,"DEBUG: %d Width, %d Height\n", tfImageWidth, tfImageHeight);
    */

    /*
     * Und Diverse Tags abfragen und ggf. auf defaults setzen ...
     *
     */

    if (!TIFFGetField(tiff_file, TIFFTAG_BITSPERSAMPLE,&tfBitsPerSample)) {
      tfBitsPerSample = 1;
    }
    if (!TIFFGetField(tiff_file, TIFFTAG_SAMPLESPERPIXEL,&tfSamplesPerPixel)) {
      tfSamplesPerPixel = 1;
    }
    if (!TIFFGetField(tiff_file, TIFFTAG_PLANARCONFIG, &tfPlanarConfiguration)) {
      tfPlanarConfiguration = PLANARCONFIG_CONTIG;
    }
    if (!TIFFGetField(tiff_file, TIFFTAG_GRAYRESPONSEUNIT, &tfGrayResponseUnit)) {
      tfGrayResponseUnit = 2;
    }

    if (!TIFFGetField(tiff_file, TIFFTAG_XRESOLUTION, &tfXRES)) {
      tfXRES=(double) -1.0;
    }

    if (!TIFFGetField(tiff_file, TIFFTAG_YRESOLUTION, &tfYRES)) {
      tfYRES=(double) -1.0;
    }

    /*
     * !!! Hier noch eine Abfrage, ob alle TAGs wie erwartet und ggf. Fehler
     * bzw return(NULL).
     *
     */

    tfImageDepth = tfBitsPerSample * tfSamplesPerPixel;

    if (!TIFFGetField(tiff_file, TIFFTAG_PHOTOMETRIC, &tfPhotometricInterpretation)) {
      tfPhotometricInterpretation = PHOTOMETRIC_MINISBLACK;
    }
    tfBytesPerRow = TIFFScanlineSize(tiff_file);

    /*
    fprintf(stderr,"BytesPerRow: %d\n", tfBytesPerRow);
    */

    rc->orow=-1;
    rc->ocol=-1;
    rc->h=tfImageHeight;
    rc->w=tfImageWidth;
    rc->rowbytes=tfBytesPerRow;
    rc->xres = tfXRES;
    rc->yres = tfYRES;

    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      TIFFClose(tiff_file);
      return(NULL);
    }

    /*
     * und einlesen, oder ?!
     *
     */

    for(i=0; i<(int)tfImageHeight;i++) {
      if (TIFFReadScanline(tiff_file, (rc->data)+(i*rc->rowbytes), i, 0) < 0) {
	fprintf(stderr,"unexpected end of scanlines ...\n");
	break;
      }
    }
    TIFFClose(tiff_file);
    return(rc);
  }
}

int bitmap_npages(char* filename) {
  TIFF* tiff_file;
  int directory;
  if((tiff_file = TIFFOpen(filename,"r")) == NULL) {
    fprintf(stderr,"WARNING: cannot open %s as TIFF.\n",filename);
    return(SCHEME_ERROR);
  }
  for(directory=1;;directory++) {
    if(!TIFFSetDirectory(tiff_file,directory)) {
      TIFFClose(tiff_file);
      return(directory);
    }
  }
}

void bitmap_destroy(BITMAP* bitmap) {
  /*
  printf ("freeing bitmap\n");
  */
  if(bitmap->data != NULL) {
    SchemeFree(bitmap->data);
  }
  SchemeFree(bitmap);
}

void greymap_destroy(GREYMAP* greymap) {
  if(greymap->data != NULL) {
    SchemeFree(greymap->data);
  }
  SchemeFree(greymap);
}

void colormap_destroy(COLORMAP* colormap) {
  if(colormap->data != NULL) {
    SchemeFree(colormap->data);
  }
  SchemeFree(colormap);
}

/*
 * Write a bitmap ...
 *
 */

static TIFF* tif = NULL;

static int tif_open(char* file, int rows, int columns, int inverted, double xres, double yres, char* modestring) {
  if(tif != NULL) {
    fprintf(stderr,"tif != NULL bei tif_open() (FATAL)\n");
    exit(2);
  }
  tif = TIFFOpen(file, modestring); 
  if(tif == NULL) {
    return(SCHEME_ERROR);
  }
  TIFFSetField( tif, TIFFTAG_IMAGEWIDTH, columns);      /* columns */
  TIFFSetField( tif, TIFFTAG_IMAGELENGTH, rows);     	/* rows */
  TIFFSetField( tif, TIFFTAG_BITSPERSAMPLE, 1);
  TIFFSetField( tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
  TIFFSetField( tif, TIFFTAG_COMPRESSION, COMPRESSION_CCITTFAX4);
  /*
  TIFFSetField( tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
  */
  if(inverted) {
    TIFFSetField( tif, TIFFTAG_PHOTOMETRIC,  PHOTOMETRIC_MINISWHITE);
  } else {
    TIFFSetField( tif, TIFFTAG_PHOTOMETRIC,  PHOTOMETRIC_MINISBLACK);
  }
  TIFFSetField( tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
  /*
  TIFFSetField( tif, TIFFTAG_DOCUMENTNAME, file);
  TIFFSetField( tif, TIFFTAG_IMAGEDESCRIPTION, "scheme" );
  */
  TIFFSetField( tif, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField( tif, TIFFTAG_ROWSPERSTRIP, rows);
  TIFFSetField( tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  if(xres > 0) {
    TIFFSetField( tif, TIFFTAG_XRESOLUTION, (float) xres);
  }
  if(yres > 0) {
    TIFFSetField( tif, TIFFTAG_YRESOLUTION, (float) yres);
  }
  return(SCHEME_OK);
}

static tif_newpage(char* file, int rows, int columns) {
  TIFFWriteDirectory(tif);
  TIFFSetField( tif, TIFFTAG_IMAGEWIDTH, columns);      /* columns */
  TIFFSetField( tif, TIFFTAG_IMAGELENGTH, rows);        /* rows */
  TIFFSetField( tif, TIFFTAG_BITSPERSAMPLE, 1);
  TIFFSetField( tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
  TIFFSetField( tif, TIFFTAG_COMPRESSION, COMPRESSION_CCITTFAX4);
  TIFFSetField( tif, TIFFTAG_PHOTOMETRIC,  PHOTOMETRIC_MINISBLACK);
  TIFFSetField( tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
  TIFFSetField( tif, TIFFTAG_DOCUMENTNAME, file);
  TIFFSetField( tif, TIFFTAG_IMAGEDESCRIPTION, "scheme" );
  TIFFSetField( tif, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField( tif, TIFFTAG_ROWSPERSTRIP, rows);
  TIFFSetField( tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
}

static tif_write(unsigned char* buf, int length, int row) {
  if(tif == NULL) {
    fprintf(stderr,"tif == NULL bei tif_write() (FATAL)\n");
    exit(2);
  }
  /*
  if(TIFFWriteEncodedStrip(tif, 0, buf, length) < 0) {
  if(TIFFWriteRawStrip(tif, 0, buf, length) < 0) {
  */
  /*
  if(TIFFWriteEncodedStrip(tif, 0, buf, length) < 0) {
    fprintf(stderr,"error writing rawstrip via TIFFWriteRawStrip\n");
    exit(2);
  }
  */
  TIFFWriteScanline(tif,buf,row,1);
}

static tif_close() {
  if(tif == NULL) {
    fprintf(stderr,"tif == NULL bei tif_close() (FATAL)\n");
    exit(2);
  }
  TIFFClose(tif);
  tif=NULL;
}

/*
 *
 * Und die Funktion:
 *
 */

int bitmap_writetiff(BITMAP* bitmap, char* filename) {
  int i;
  if(tif_open(filename, 
	      bitmap->h, 
	      bitmap->w, 1, bitmap->xres, bitmap->yres,"w") == SCHEME_OK) {
    for(i=0; i<bitmap->h; i++) {
      tif_write(((unsigned char*) 
	         (bitmap->data))+((bitmap->rowbytes)*i), 
	         bitmap->rowbytes, i);
    }
    tif_close();
    return(SCHEME_OK);
  } else {
    return(SCHEME_ERROR);
  }
}

#define BYTES_PER_OUTPUT_LINE 12

static int dt[] = {
  /* 0000 0000 */ '0',
  /* 0001 1000 */ '8',
  /* 0010 0100 */ '4',
  /* 0011 1100 */ 'c', 
  /* 0100 0010 */ '2',
  /* 0101 1010 */ 'a',
  /* 0110 0110 */ '6',
  /* 0111 1110 */ 'e',
  /* 1000 0001 */ '1',
  /* 1001 1001 */ '9',
  /* 1010 0101 */ '5',
  /* 1011 1101 */ 'd',
  /* 1100 0011 */ '3',
  /* 1101 1011 */ 'b', 
  /* 1110 0111 */ '7',
  /* 1111 1111 */ 'f' 
};

BITMAP* bitmap_readxbm(char* filename) {
  BITMAP* rc;
  int i,x_hot,y_hot;

  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=-1;
    rc->ocol=-1;
    rc->xres=-1.0;
    rc->yres=-1.0;
    if (SchemeReadXbm(filename, &rc->w, &rc->h, &rc->data, &x_hot, &y_hot) == SCHEME_OK) {
      rc->rowbytes=(rc->w+7)/8;
      return(rc);
    } else {
      SchemeFree(rc);
      return(NULL);
    }
  }
}

int bitmap_writexbm(BITMAP* bitmap, char* filename) {
  unsigned char *data;
  unsigned char *ptr;
  int size, byte,r,c,cc;
  int width, height;
  FILE *stream;
  char *name;

  if (!(name = strrchr(filename, '/')))
    name = (char *)filename;
  else
    name++;

  if (!(stream = fopen(filename, "w")))
     return(SCHEME_ERROR);

  data=(unsigned char*) bitmap->data;
  size=bitmap->h*bitmap->rowbytes;
  width=bitmap->w;
  height=bitmap->h;
  byte=0;

  fprintf(stream, "#define %s_width %d\n", name, width);
  fprintf(stream, "#define %s_height %d\n", name, height);
  fprintf(stream, "static unsigned char %s_bits[] = {", name);

  for(r=0; r < bitmap->h; r++) {
    for(c=0; c < bitmap->rowbytes; c++) {
      if (!byte)
        fprintf(stream, "\n   ");
      else if (!(byte % BYTES_PER_OUTPUT_LINE))
        fprintf(stream, ",\n   ");
      else
        fprintf(stream, ", ");
      cc=(*(((unsigned char*) bitmap->data)+((bitmap->rowbytes)*r)+c));
      fprintf(stream, "0x%c%c", dt[cc&0x0f], dt[(cc>>4)&0x0f] );
      byte++;
    }
  }

  fprintf(stream, "};\n");
  fclose(stream);

  return(SCHEME_OK);
}

int bitmap_appendtiff(BITMAP* bitmap, char* filename) {
  int i;
  if(tif_open(filename, 
	      bitmap->h, 
	      bitmap->w, 
	      1, bitmap->xres, bitmap->yres,"a") == SCHEME_OK) {
    for(i=0; i<bitmap->h; i++) {
      tif_write(((unsigned char*) 
	         (bitmap->data))+((bitmap->rowbytes)*i), 
	         bitmap->rowbytes, i);
    }
    tif_close();
    return(SCHEME_OK);
  } else {
    return(SCHEME_FALSE);
  }
}

void bitmap_invert(BITMAP* bitmap) {
  int r,c;
  for(r=0; r < bitmap->h; r++) {
    for(c=0; c < bitmap->rowbytes; c++) {
      *((bitmap->data)+((bitmap->rowbytes)*r)+c) = 
	~(*((bitmap->data)+((bitmap->rowbytes)*r)+c));
    }
  }
}

void greymap_invert(GREYMAP* greymap) {
  int r,c;
  for(r=0; r < greymap->h; r++) {
    for(c=0; c < greymap->w; c++) {
      (*((greymap)->data+(r)*((greymap)->w)+(c))) =
      ~(*((greymap)->data+(r)*((greymap)->w)+(c)));
    }
  }
}


void bitmap_grey(BITMAP* bitmap) {
  int r,c;
  for(r=0; r < bitmap->h; r++) {
    for(c=0; c < bitmap->rowbytes; c++) {
      if(r%2) {
        *((bitmap->data)+((bitmap->rowbytes)*r)+c) &= 0xaa;
      } else {
        *((bitmap->data)+((bitmap->rowbytes)*r)+c) &= 0x55;
      }
    }
  }
}

void bitmap_hstripe(BITMAP* bitmap) {
  int r,c;
  for(r=0; r < bitmap->h; r++) {
    for(c=0; c < bitmap->rowbytes; c++) {
      if(r%2) {
        *((bitmap->data)+((bitmap->rowbytes)*r)+c) = 0x00;
      }
    }
  }
}

void bitmap_vstripe(BITMAP* bitmap) {
  int r,c;
  for(r=0; r < bitmap->h; r++) {
    for(c=0; c < bitmap->rowbytes; c++) {
      *((bitmap->data)+((bitmap->rowbytes)*r)+c) &= 0xaa;
    }
  }
}


int bitmap_getpixel(BITMAP* bitmap, int x, int y) {
  if(x < 0 || x >= bitmap->h || y < 0 || y >= bitmap->w) {
    return(SCHEME_ERROR); 
  } else {
    /*
    int bn,bb,byte,mask;
    bn=y/8;
    bb=y%8;
    byte=*((bitmap->data)+((bitmap->rowbytes)*x)+bn);
    mask=0x80 >> bb;
    if(byte & mask) {
      return(1);
    } else {
      return(0);
    }
    */
    if(GETPIXEL(bitmap,x,y)) {
      return(1);
    } else {
      return(0);
    }
  }
}

int greymap_getpixel(GREYMAP* greymap, int x, int y) {
  if(x < 0 || x >= greymap->h || y < 0 || y >= greymap->w) {
    return(SCHEME_ERROR); 
  } else {
    return((unsigned int) GREYMAP_GETPIXEL(greymap,x,y));
  }
}

/*
 * Bresenham's algorithm (aus Computer Graphics, August 1996, Nummer 3)
 */

int bitmap_drawline(BITMAP* bitmap, int x1, int y1, int x2, int y2) {
  if(x1 < 0 || x1 >= bitmap->h || y1 < 0 || y1 >= bitmap->w ||
     x2 < 0 || x2 >= bitmap->h || y2 < 0 || y2 >= bitmap->w) {
    return(SCHEME_ERROR); 
  } else {
    int xt, yt;
    int dx, dy, x, y_int, error, inc1, inc2;
    if(x1 > x2) {
      xt=x1; yt=y1; x1=x2; y1=y2; x2=xt; y2=yt;
    }
    dx=x2-x1;
    dy=y2-y1;
    error=2*dy-dx;
    inc1=2*dy;
    inc2=2*(dy-dx);
    y_int=y1;
    for(x=x1; x<=x2; x++) {
      SETPIXEL(bitmap,y_int,x);
      if(error<0) {
	error=error+inc1;
      } else {
	error=error+inc2;
	y_int=y_int+1;
      }
    } 
    return(SCHEME_OK);
  }
}

int bitmap_setpixel(BITMAP* bitmap, int x, int y) {
  if(x < 0 || x >= bitmap->h || y < 0 || y >= bitmap->w) {
    return(SCHEME_ERROR);
  } else {
    int bn,bb,byte,mask;
    bn=y/8;
    bb=y%8;
    byte=*((bitmap->data)+((bitmap->rowbytes)*x)+bn);
    mask=0x80 >> bb;
    *((bitmap->data)+((bitmap->rowbytes)*x)+bn)=byte|mask;
    return(SCHEME_OK);
  }
}

int greymap_setpixel(GREYMAP* greymap, int x, int y, int v) {
  if(x < 0 || x >= greymap->h || y < 0 || y >= greymap->w) {
    return(SCHEME_ERROR);
  } else {
    GREYMAP_GETPIXEL(greymap,x,y) = v;
    return(SCHEME_OK);
  }
}

int bitmap_clearpixel(BITMAP* bitmap, int x, int y) {
  if(x < 0 || x >= bitmap->h || y < 0 || y >= bitmap->w) {
    return(SCHEME_ERROR);
  } else {
    int bn,bb,byte,mask;
    bn=y/8;
    bb=y%8;
    byte=*((bitmap->data)+((bitmap->rowbytes)*x)+bn);
    mask=~(0x80 >> bb);
    *((bitmap->data)+((bitmap->rowbytes)*x)+bn)=byte&mask;
    return(SCHEME_OK);
  }
}

static char ctable[] = {
  /* 0000 */ 0,
  /* 0001 */ 0,
  /* 0010 */ 1,
  /* 0011 */ 1,
  /* 0100 */ 0,
  /* 0101 */ 0,
  /* 0110 */ 1,
  /* 0111 */ 1,
  /* 1000 */ 2,
  /* 1001 */ 2,
  /* 1010 */ 3,
  /* 1011 */ 3,
  /* 1100 */ 2,
  /* 1101 */ 2,
  /* 1110 */ 3,
  /* 1111 */ 3
};

BITMAP* bitmap_half(BITMAP* b) {
  BITMAP* rc;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    int row, nb, val;
    int b1,b2;
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    if(b->xres>=0)
      rc->xres=b->xres/2;
    if(b->yres>=0)
      rc->yres=b->yres/2;
    rc->h=(b->h)/2;
    rc->w=(b->w)/2;
    rc->rowbytes=(rc->w+7)/8;
    if(rc->h == 0 || rc->w ==0 ) {
      SchemeFree(rc);
      return(NULL);
    }
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    for(row=0; row<rc->h; row++) {
      for(nb=0; nb<rc->rowbytes; nb++) {
	b1= *((b->data)+((b->rowbytes)*(row*2))+(nb*2));
	b2= *((b->data)+((b->rowbytes)*(row*2))+(nb*2)+1);
	val= ctable[(b1&0xf0)>>4]<<6 |
	     ctable[b1&0x0f]<<4 |
	     ctable[(b2&0xf0)>>4]<<2 |
	     ctable[b2&0x0f];
        *((rc->data)+((rc->rowbytes)*row)+nb)=val;
      }
    }
    return(rc);
  }
}

BITMAP* bitmap_half4(BITMAP* b) {
  BITMAP* rc;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    int row, nb, val;
    int b1,b2,b3,b4;
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    rc->h=(b->h)/2;
    rc->w=(b->w)/2;
    if(b->xres>=0)
      rc->xres=b->xres/2;
    if(b->yres>=0)
      rc->yres=b->yres/2;
    rc->rowbytes=(rc->w+7)/8;
    if(rc->h == 0 || rc->w ==0 ) {
      SchemeFree(rc);
      return(NULL);
    }
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    for(row=0; row<rc->h; row++) {
      for(nb=0; nb<rc->rowbytes; nb++) {
	b1= *((b->data)+((b->rowbytes)*(row*2))+(nb*2));
	b2= *((b->data)+((b->rowbytes)*(row*2))+(nb*2)+1);
	b3= *((b->data)+((b->rowbytes)*(row*2+1))+(nb*2));
	b4= *((b->data)+((b->rowbytes)*(row*2+1))+(nb*2)+1);
	val= 
         (((b1 & 0xc0) || (b3 & 0xc0)) ? 0x80 : 0x00) |
         (((b1 & 0x30) || (b3 & 0x30)) ? 0x40 : 0x00) |
         (((b1 & 0x0c) || (b3 & 0x0c)) ? 0x20 : 0x00) |
         (((b1 & 0x03) || (b3 & 0x03)) ? 0x10 : 0x00) |
         (((b2 & 0xc0) || (b4 & 0xc0)) ? 0x08 : 0x00) |
         (((b2 & 0x30) || (b4 & 0x30)) ? 0x04 : 0x00) |
         (((b2 & 0x0c) || (b4 & 0x0c)) ? 0x02 : 0x00) |
         (((b2 & 0x03) || (b4 & 0x03)) ? 0x01 : 0x00);
        *((rc->data)+((rc->rowbytes)*row)+nb)=val;
      }
    }
    return(rc);
  }
}

static int dtable[] = {
  /* 0000 */ 0x00,
  /* 0001 */ 0x03,
  /* 0010 */ 0x0c,
  /* 0011 */ 0x0f, 
  /* 0100 */ 0x30,
  /* 0101 */ 0x33,
  /* 0110 */ 0x3c,
  /* 0111 */ 0x3f,
  /* 1000 */ 0xc0,
  /* 1001 */ 0xc3,
  /* 1010 */ 0xcc,
  /* 1011 */ 0xcf,
  /* 1100 */ 0xf0,
  /* 1101 */ 0xf3,
  /* 1110 */ 0xfc,
  /* 1111 */ 0xff
};

BITMAP* bitmap_double(BITMAP* b) {
  BITMAP* rc;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    int row, nb, val;
    int bb,bx,b1,b2;
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    rc->h=(b->h)*2;
    rc->w=(b->w)*2;
    if(b->xres>=0)
      rc->xres=b->xres*2;
    else 
      rc->xres=-1;
    if(b->yres>=0)
      rc->yres=b->yres*2;
    else 
      rc->yres=-1;
    rc->rowbytes=(rc->w+7)/8;
    if(rc->h == 0 || rc->w ==0 ) {
      SchemeFree(rc);
      return(NULL);
    }
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    for(row=0; row<b->h; row++) {
      for(nb=0; nb<b->rowbytes; nb++) {
        bx=*((b->data)+((b->rowbytes)*(row))+(nb));
	b1=dtable[(bx&0xf0)>>4];
	b2=dtable[(bx&0x0f)];
	if(nb*2 < rc->rowbytes) {
	  *((rc->data)+((rc->rowbytes)*(row*2))+(nb*2))=b1;
	  *((rc->data)+((rc->rowbytes)*(row*2+1))+(nb*2))=b1;
	}
	if(nb*2+1 < rc->rowbytes) {
	  *((rc->data)+((rc->rowbytes)*(row*2))+(nb*2)+1)=b2;
	  *((rc->data)+((rc->rowbytes)*(row*2+1))+(nb*2)+1)=b2;
	}
      }
    }
    return(rc);
  }
}

BITMAP* bitmap_vcdouble(BITMAP* b) {
  BITMAP* rc;
  int row,col;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    int row, nb, val;
    int bb,bx,b1,b2;
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    rc->h=(b->h)*2;
    rc->w=(b->w)*2;
    if(b->xres>=0)
      rc->xres=b->xres*2;
    else 
      rc->xres=-1;
    if(b->yres>=0)
      rc->yres=b->yres*2;
    else 
      rc->yres=-1;
    rc->rowbytes=(rc->w+7)/8;
    if(rc->h == 0 || rc->w ==0 ) {
      SchemeFree(rc);
      return(NULL);
    }
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->rowbytes);
    for(row=0; row<b->h; row++) {
      for(col=0; col<b->w; col++) {
        if(GETPIXEL(b,row,col)) {
          SETPIXEL(rc,row*2+0,col*2+0);
          SETPIXEL(rc,row*2+1,col*2+1);
        } else {
          SETPIXEL(rc,row*2+0,col*2+1);
          SETPIXEL(rc,row*2+1,col*2+0);
        }
      }
    }
    return(rc);
  }
}

int bitmap_compare(BITMAP* a, BITMAP* b, int *equalpix, int *totalpix) {
  int row, col, rowmin, colmin, rowmax, colmax;
  int epix, tpix;
  epix=0;
  rowmin = a->h <= b->h ? a->h : b->h;
  colmin = a->w <= b->w ? a->w : b->w;
  rowmax = a->h >= b->h ? a->h : b->h;
  colmax = a->w >= b->w ? a->w : b->w;
  tpix=rowmax*colmax;
  for (row = 0; row < rowmin; row++) {
    for(col = 0; col < colmin; col++) {
      if(GETPIXEL(a,row,col) == GETPIXEL(b,row,col)) {
	epix++;
      }
    }
  }
  *equalpix=epix;
  *totalpix=tpix;
  return(SCHEME_OK);
}

int greymap_equal(GREYMAP* a, GREYMAP* b) {
  int row, col;
  if(a->h != b-> h || a->w != b->w) {
    return(SCHEME_FALSE);
  } else {
    for(row=0; row<a->h; row++) {
      for(col=0; col<a->w; col++) {
	if(GREYMAP_GETPIXEL(a,row,col) != GREYMAP_GETPIXEL(b,row,col)) {
	  return(SCHEME_FALSE);
	}
      }
    }
    return(SCHEME_TRUE);
  }
}

int bitmap_equal(BITMAP* a, BITMAP* b) {
  static unsigned char map[]={0xff, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};
  int rowbyte, col;
  if(a->h != b-> h || a->w != b->w) {
    return(SCHEME_FALSE);
  } else {
    for(col=0; col<a->h; col++) {
      for(rowbyte=0; rowbyte<a->rowbytes; rowbyte++) {
	if(rowbyte==(a->rowbytes)-1) {
	  unsigned char pattern=map[(a->w)%8]; 
	  if((*(a->data+col*(a->rowbytes)+rowbyte)&pattern) != (*(b->data+col*(b->rowbytes)+rowbyte)&pattern)) {
	    return(SCHEME_FALSE);
	  }
	} else {
	  if(*(a->data+col*a->rowbytes+rowbyte) != *(b->data+col*b->rowbytes+rowbyte)) {
	    return(SCHEME_FALSE);
	  }
	}
      }
    }
    return(SCHEME_TRUE);
  }
}

int bitmap_fold(BITMAP* a, int hashsize) {
  static unsigned char map[]={0xff, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};
  unsigned int rc=0;
  int rowbyte, col;

  rc+=a->h+31*rc;
  rc+=a->w+31*rc;

  for(col=0; col<a->h; col++) {
    for(rowbyte=0; rowbyte<a->rowbytes; rowbyte++) {
      if(rowbyte==(a->rowbytes)-1) {
	unsigned char pattern=map[(a->w)%8]; 
	rc=(*(a->data+col*(a->rowbytes)+rowbyte)&pattern)+31*rc;
      } else {
	rc=(*(a->data+col*a->rowbytes+rowbyte))+31*rc;
      }
    }
  }
  return(rc % hashsize);
}

int bitmap_nextblack(BITMAP* a, int startrow, int startcol, int *retrow, int *retcol) {
  int row, col;
  for (row = startrow; row < a->h; row++) {
    if(row == startrow) {
      for(col = startcol; col < a->w; col++) {
        if(GETPIXEL(a,row,col)) {
	  *retrow=row;
	  *retcol=col;
	  return(SCHEME_OK);
        }
      }
    } else {
      for(col = 0; col < a->w; col++) {
        if(GETPIXEL(a,row,col)) {
	  *retrow=row;
	  *retcol=col;
	  return(SCHEME_OK);
        }
      }
    }
  }
  return(SCHEME_OK);
}

int bitmap_nextblackup(BITMAP* a, int startrow, int startcol, int *retrow, int *retcol) {
  int row, col;
  for (row = startrow; row >= 0; row--) {
    if(row == startrow) {
      for(col = startcol; col < a->w; col++) {
        if(GETPIXEL(a,row,col)) {
	  *retrow=row;
	  *retcol=col;
	  return(SCHEME_OK);
        }
      }
    } else {
      for(col = 0; col < a->w; col++) {
        if(GETPIXEL(a,row,col)) {
	  *retrow=row;
	  *retcol=col;
	  return(SCHEME_OK);
        }
      }
    }
  }
  return(SCHEME_OK);
}

static BITMAP* source;
static BITMAP* destination=NULL;
static int minrow, maxrow, mincol, maxcol;

/* The following code is derived from the function FloodLoop() in the 
 * file "Graphics.c" of the Program "bitmap" so the following Copyright
 * and permission notice applies to the function "crunch2":
 *
 * Copyright (c) 1989  X Consortium
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

typedef int Position;
typedef int Dimension;
typedef int Boolean;

#define QueryFlood(ignored1,x,y,ignored2) ((x<0 || x>=source->h || y<0 || y>=source->w)?(0):(GETPIXEL(source,x,y)))
#define BW (1)
#define FloodLoop(a,b,c,d) crunch2((b),(c))
#define False (0)
#define True (1)

static Flood(int ignored1, int x, int y, int ignoed2) {
  if(x >= 0 && x <source->h && y >= 0 && y < source->w) {
    SETPIXEL(destination,x,y);
    CLEARPIXEL(source,x,y);
    if(x < minrow) minrow=x;
    if(x > maxrow) maxrow=x;
    if(y < mincol) mincol=y;
    if(y > maxcol) maxcol=y;
  }
}

static void crunch2(int x, int y) {
  Position save_x, save_y, x_left, x_right;
  int value=0;
  
  if (QueryFlood(BW, x, y, value)) 
      Flood(BW, x, y, value);

  save_x = x;
  save_y = y;

  x++;
  while (QueryFlood(BW, x, y, value)) {
      Flood(BW, x, y, value);
      x++;
  }
  x_right = --x;

  x = save_x;
  x--;
  while (QueryFlood(BW, x, y, value)) {
      Flood(BW, x, y, value);
      x--;
  }
  x_left = ++x;

  x = x_left;
  y = save_y;
  y++;
  
  while (x <= x_right) {
      Boolean flag = False;
      Position x_enter;
      
      while (QueryFlood(BW, x, y, value) && (x <= x_right)) {
	  flag = True;
	  x++;
      }
      
      if (flag) {
	  if ((x == x_right) && QueryFlood(BW, x, y, value))
	      FloodLoop(BW, x, y, value);
	  else
	      FloodLoop(BW, x - 1, y, value);
      }
      
      x_enter = x;
      
      while (!QueryFlood(BW, x, y, value) && (x < x_right))
	  x++;
      
      if (x == x_enter) x++;
  }

  x = x_left;
  y = save_y;
  y--;

  while (x <= x_right) {
      Boolean flag = False;
      Position x_enter;
      
      while (QueryFlood(BW, x, y, value) && (x <= x_right)) {
	  flag = True;
	  x++;
      }
      
      if (flag) {
	  if ((x == x_right) && QueryFlood(BW, x, y, value))
	      FloodLoop(BW, x, y, value);
	  else
	      FloodLoop(BW, x - 1, y, value);
      }
      
      x_enter = x;
      
      while (!QueryFlood(BW, x, y, value) && (x < x_right))
	  x++;
      
      if (x == x_enter) x++;
  }
}


BITMAP* bitmap_extract(BITMAP* bm, int r, int c) {
  BITMAP* rc=NULL;
  int rcrow, rccol;

  if(!GETPIXEL(bm,r,c)) {
    return(NULL);
  }

  if(r < 0 || c < 0 || r >= bm->h || c >= bm->w) {
    return(NULL);
  }
 
  if(destination == NULL) {
    destination = bitmap_create(bm->h, bm->w); 
  } else {
    if(destination->h != bm->h || destination->w != bm->w) {
      bitmap_destroy(destination);
      destination=NULL;
      destination = bitmap_create(bm->h, bm->w); 
    } 
   /* else {
      bzero(destination->data, destination->h*destination->rowbytes);
    }
    */
  }

  source = bm;

  minrow=bm->h; maxrow=-1; 
  mincol=bm->w; maxcol=-1;

  crunch2(r,c);   

  if(maxrow==-1) {
    rc=NULL;
  } else {
    int i,rr,cc;
    rcrow=maxrow-minrow+1;
    rccol=maxcol-mincol+1;

    rc=bitmap_create(rcrow,rccol);
    for(rr=0; rr<rc->h; rr++) {
      for(cc=0; cc<rc->w; cc++) {
	if(GETPIXEL(destination, minrow+rr, mincol+cc)) {
	  SETPIXEL(rc,rr,cc);
	  CLEARPIXEL(destination, minrow+rr, mincol+cc);
	}  
      }
    }
  }
  /*
  bitmap_destroy(destination);
  */
  rc->ocol=mincol;
  rc->orow=minrow;
  rc->xres=bm->xres;
  rc->yres=bm->yres;
  return(rc);
}

int bitmap_implant(BITMAP* subbitmap, BITMAP* bitmap) {
  int r,c;
  if(subbitmap->ocol == -1 || subbitmap->orow == -1) {
    return(SCHEME_ERROR);
  } else {
    for(r=subbitmap->orow; r < subbitmap->h+subbitmap->orow && r < bitmap->h; r++) {
      for(c=subbitmap->ocol; c < subbitmap->w+subbitmap->ocol && c < bitmap->w; c++) {
	if(GETPIXEL(subbitmap, r-subbitmap->orow, c-subbitmap->ocol)) {
	  SETPIXEL(bitmap,r,c);
	}
      }
    }
  }
  return(SCHEME_OK);
}

int bitmap_implantxor(BITMAP* subbitmap, BITMAP* bitmap) {
  int r,c;
  if(subbitmap->ocol == -1 || subbitmap->orow == -1) {
    return(SCHEME_ERROR);
  } else {
    for(r=subbitmap->orow; r < subbitmap->h+subbitmap->orow && r < bitmap->h; r++) {
      for(c=subbitmap->ocol; c < subbitmap->w+subbitmap->ocol && c < bitmap->w; c++) {
	if(GETPIXEL(subbitmap, r-subbitmap->orow, c-subbitmap->ocol) ^ GETPIXEL(bitmap,r,c)) {
	  SETPIXEL(bitmap,r,c);
	} else {
	  CLEARPIXEL(bitmap,r,c);
	}
      }
    }
  }
  return(SCHEME_OK);
}

BITMAP* bitmap_rrotate(BITMAP* b) {
  BITMAP* rc;
  int c,r;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    rc->h=b->w;
    rc->w=b->h;
    rc->xres=b->yres;
    rc->yres=b->xres;
    rc->rowbytes=(rc->w+7)/8;
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->rowbytes);
    for(r=0; r<b->h; r++) {
      for(c=0; c<b->w; c++) {
	if(GETPIXEL(b,r,c)) {
	  SETPIXEL(rc,c,rc->w-r-1);
	}
      }
    }
    return(rc);
  }
}

BITMAP* bitmap_lrotate(BITMAP* b) {
  BITMAP* rc;
  int c,r;
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    rc->h=b->w;
    rc->w=b->h;
    rc->xres=b->yres;
    rc->yres=b->xres;
    rc->rowbytes=(rc->w+7)/8;
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->rowbytes);
    for(r=0; r<b->h; r++) {
      for(c=0; c<b->w; c++) {
	if(GETPIXEL(b,r,c)) {
	  SETPIXEL(rc,rc->h-c-1,r);
	}
      }
    }
    return(rc);
  }
}

typedef struct { 
  Position *x, *y;
  Dimension *width, *height; 
} Table;

#define mymax(x, y) (((int)(x) > (int)(y)) ? (x) : (y))  
#define myrint(x)   floor(x + 0.5)

BITMAP* bitmap_scale(BITMAP* b, double scale_x, double scale_y) {
  BITMAP* rc;
  Dimension width, height;
  Table table;
  Position x, y, w, h;
  int pixel;

  width = mymax(myrint(scale_x * b->w), 1);
  height = mymax(myrint(scale_y * b->h), 1);

  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=b->orow; /* original row und col werden behalten */
    rc->ocol=b->ocol;
    rc->h=height;
    rc->w=width;

    if(b->xres >= 0.0)
      rc->xres=(b->xres * scale_x);
    else
      rc->xres=b->xres;

    if(b->yres >= 0.0)
      rc->yres=(b->yres * scale_y);
    else
      rc->yres=b->yres;

    rc->rowbytes=(width+7)/8;

    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->rowbytes);
  }

  if(width == b->w && height == b->h) {
    memcpy(rc->data, b->data, rc->rowbytes*rc->h);
  } else {
    table.x = (Position *) SchemeMalloc(sizeof(Position) * b->w);
    table.y = (Position *) SchemeMalloc(sizeof(Position) * b->h);
    table.width = (Dimension *) SchemeMalloc(sizeof(Dimension) * b->w);
    table.height = (Dimension *) SchemeMalloc(sizeof(Dimension) * b->h);

    for (x = 0; x < b->w; x++) { 
      table.x[x] = myrint(scale_x * x);
      table.width[x] = myrint(scale_x * (x + 1)) - myrint(scale_x * x);
    }
    for (y = 0; y < b->h; y++) {
      table.y[y] = myrint(scale_y * y);
      table.height[y] = myrint(scale_y * (y + 1)) - myrint(scale_y * y);
    }
 
    for (x = 0; x < b->w; x++)
      for (y = 0; y < b->h; y++) {
        pixel = GETPIXEL(b, y, x);
	for (w = 0; (int)w < (int)table.width[x]; w++)
	  for (h = 0; (int)h < (int)table.height[y]; h++)
	    if (pixel) SETPIXEL(rc, table.y[y] + h, table.x[x] + w); 
       }

    SchemeFree((char *)table.x);
    SchemeFree((char *)table.y);
    SchemeFree((char *)table.width);
    SchemeFree((char *)table.height);
  }
  return rc;
}

BITMAP* bitmap_scaleabsolute(BITMAP* b, int x, int y) {
  double sx, sy;
  sx=(double) x / (double) b->w;
  sy=(double) y / (double) b->h;
  return(bitmap_scale(b,sx,sy));
}

BITMAP* bitmap_crop(BITMAP* b, int r, int c, int h, int w) {
  int rr, cc;
  BITMAP* rc;
  if(r < 0 || r >= b->h || c < 0 || c >= b->w || h < 0 || w < 0 ||
     r+h > b->h || c+w > b->w) {
    return(NULL);
  }
  rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP)); 
  rc->orow=r;
  rc->ocol=c;
  rc->h=h;
  rc->w=w;
  rc->xres=b->xres;
  rc->yres=b->yres;
  rc->rowbytes=(w+7)/8;
  if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
    SchemeFree(rc);
    return(NULL);
  }
  bzero(rc->data, rc->h*rc->rowbytes);
  for(rr = r; rr < r+h; rr++) {
    for(cc = c; cc < c+w; cc++) {
      if(GETPIXEL(b, rr, cc)) {
	SETPIXEL(rc,rr - r, cc - c);
      }
    }
  }
  return(rc);
}

GREYMAP* greymap_crop(GREYMAP* b, int r, int c, int h, int w) {
  int rr, cc;
  GREYMAP* rc;
  if(r < 0 || r >= b->h || c < 0 || c >= b->w || h < 0 || w < 0 ||
     r+h > b->h || c+w > b->w) {
    return(NULL);
  }
  rc = (GREYMAP*) SchemeMalloc(sizeof(GREYMAP)); 
  rc->orow=r;
  rc->ocol=c;
  rc->h=h;
  rc->w=w;
  rc->xres=b->xres;
  rc->yres=b->yres;
  if((rc->data = (unsigned char*) SchemeMalloc(rc->h*rc->w)) == NULL) {
    SchemeFree(rc);
    return(NULL);
  }
  bzero(rc->data, rc->h*rc->w);
  for(rr = r; rr < r+h; rr++) {
    for(cc = c; cc < c+w; cc++) {
      GREYMAP_GETPIXEL(rc, rr - r, cc - c) = GREYMAP_GETPIXEL(b, rr, cc);
    }
  }
  return(rc);
}

GREYMAP* greymap_smooth_region(GREYMAP* b, int r, int c, int h, int w, int size) {
  int rr, cc, rrr, ccc;
  int halfsize=size/2;
  GREYMAP* rc;
  if(r < 0 || r >= b->h || c < 0 || c >= b->w || h < 0 || w < 0 ||
     r+h > b->h || c+w > b->w) {
    return(NULL);
  }
  if(size == halfsize*2 || size <= 0) {
    return(NULL);
  }
  rc = (GREYMAP*) SchemeMalloc(sizeof(GREYMAP)); 
  rc->orow=r;
  rc->ocol=c;
  rc->h=h;
  rc->w=w;
  rc->xres=b->xres;
  rc->yres=b->yres;
  if((rc->data = (unsigned char*) SchemeMalloc(rc->h*rc->w)) == NULL) {
    SchemeFree(rc);
    return(NULL);
  }
  bzero(rc->data, rc->h*rc->w);
  for(rr = r; rr < r+h; rr++) {
    for(cc = c; cc < c+w; cc++) {
      int value=0;
      for(ccc = cc-halfsize; ccc<= cc+halfsize; ccc++) {
	for(rrr = rr-halfsize; rrr <= rr+halfsize; rrr++) {
	  if(ccc >= 0 && ccc < b->w && rrr >= 0 && rrr < b->h) {
	    value+=GREYMAP_GETPIXEL(b, rrr, ccc);
	  }
	}
      }
      value=value/(size*size);
      GREYMAP_GETPIXEL(rc, rr - r, cc - c) = value;
    }
  }
  return(rc);
}

GREYMAP* greymap_scale(GREYMAP* b, double scale_x, double scale_y) {
  GREYMAP* rc;
  Dimension width, height;
  Table table;
  Position x, y, w, h;
  int pixel;

  width = mymax(myrint(scale_x * b->w), 1);
  height = mymax(myrint(scale_y * b->h), 1);

  if ((rc = (GREYMAP*) SchemeMalloc(sizeof(GREYMAP))) == NULL) {
    return(NULL);
  } else {
    rc->orow=b->orow; /* original row und col werden behalten */
    rc->ocol=b->ocol;
    rc->h=height;
    rc->w=width;

    if(b->xres >= 0.0)
      rc->xres=(b->xres * scale_x);
    else
      rc->xres=b->xres;

    if(b->yres >= 0.0)
      rc->yres=(b->yres * scale_y);
    else
      rc->yres=b->yres;

    if((rc->data = (unsigned char*) SchemeMalloc(rc->h*rc->w)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }
    bzero(rc->data, rc->h*rc->w);
  }

  if(width == b->w && height == b->h) {
    memcpy(rc->data, b->data, rc->w*rc->h);
  } else {
    table.x = (Position *) SchemeMalloc(sizeof(Position) * b->w);
    table.y = (Position *) SchemeMalloc(sizeof(Position) * b->h);
    table.width = (Dimension *) SchemeMalloc(sizeof(Dimension) * b->w);
    table.height = (Dimension *) SchemeMalloc(sizeof(Dimension) * b->h);

    for (x = 0; x < b->w; x++) { 
      table.x[x] = myrint(scale_x * x);
      table.width[x] = myrint(scale_x * (x + 1)) - myrint(scale_x * x);
    }
    for (y = 0; y < b->h; y++) {
      table.y[y] = myrint(scale_y * y);
      table.height[y] = myrint(scale_y * (y + 1)) - myrint(scale_y * y);
    }
 
    for (x = 0; x < b->w; x++)
      for (y = 0; y < b->h; y++) {
        pixel = GREYMAP_GETPIXEL(b, y, x);
	for (w = 0; (int)w < (int)table.width[x]; w++)
	  for (h = 0; (int)h < (int)table.height[y]; h++)
	    GREYMAP_GETPIXEL(rc, table.y[y] + h, table.x[x] + w) = pixel; 
       }

    SchemeFree((char *)table.x);
    SchemeFree((char *)table.y);
    SchemeFree((char *)table.width);
    SchemeFree((char *)table.height);
  }
  return rc;
}

static int numbit_array[256];
static int numbit_array_done = 0;

static void do_numbit_array() {
  int i,n;
  if(numbit_array_done) {
    return;
  } else {
    for(i=0; i<256; i++) {
      n=   ((i&0x01)?1:0) +
	   ((i&0x02)?1:0) +
	   ((i&0x04)?1:0) +
	   ((i&0x08)?1:0) +
	   ((i&0x10)?1:0) +
	   ((i&0x20)?1:0) +
	   ((i&0x40)?1:0) +
	   ((i&0x80)?1:0);
      numbit_array[i]=n;
      /*
      printf("%d:%d ",i,n);
      */
    }
    /*
    printf("\n");
    */
    numbit_array_done=1;
  }
}

BITMAP* bitmap_8(BITMAP* b) {
  BITMAP* rc;
  do_numbit_array();
  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    return(NULL);
  } else {
    int row, nb, val;
    int b1,b2;
    if(b->h % 8 != 0 ||
       b->w % 8 != 0) {
      SchemeFree(rc);
      return(NULL);
    }
    rc->orow=b->orow;
    rc->ocol=b->ocol;
    if(b->xres>=0)
      rc->xres=b->xres/8;
    if(b->yres>=0)
      rc->yres=b->yres/8;
    rc->h=(b->h)/8;
    rc->w=(b->w)/8;
    rc->rowbytes=(rc->w+7)/8;
    if(rc->h == 0 || rc->w == 0 ) {
      SchemeFree(rc);
      return(NULL);
    }
    if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
      SchemeFree(rc);
      return(NULL);
    }

    {
      int nbit=0;
      for(row=0; row<b->h; row+=8) {
        for(nb=0; nb<b->rowbytes; nb++) {
          nbit = 
	    numbit_array[(unsigned int) *((unsigned char*) ((b->data)+((b->rowbytes)*((row+0)))+(nb)))] +
	    numbit_array[(unsigned int) *((unsigned char*) ((b->data)+((b->rowbytes)*((row+1)))+(nb)))] +
	    numbit_array[(unsigned int) *((unsigned char*) ((b->data)+((b->rowbytes)*((row+2)))+(nb)))] +
	    numbit_array[(unsigned int) *((unsigned char*) ((b->data)+((b->rowbytes)*((row+3)))+(nb)))] +
	    numbit_array[(unsigned int) *((unsigned char*) ((b->data)+((b->rowbytes)*((row+4)))+(nb)))] +
	    numbit_array[(unsigned int) *((unsigned char*) ((b->data)+((b->rowbytes)*((row+5)))+(nb)))] +
	    numbit_array[(unsigned int) *((unsigned char*) ((b->data)+((b->rowbytes)*((row+6)))+(nb)))] +
	    numbit_array[(unsigned int) *((unsigned char*) ((b->data)+((b->rowbytes)*((row+7)))+(nb)))];
	  /*
	  printf("%d\n",(unsigned int) *((unsigned char*) ((b->data)+((b->rowbytes)*((row+0)))+(nb))));
	  */
	  if(nbit >= 32) {
            SETPIXEL(rc,(row/8),nb);
	  } else {
            CLEARPIXEL(rc,(row/8),nb);
	  }
        }
      }
    }
    return(rc);
  }
}







