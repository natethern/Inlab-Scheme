/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: pngif.c,v 4.102 2006/04/29 09:13:22 tommy Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <scheme.h>
#include <bitmap.h>
#include <png.h>

static char* rcsid="$Id: pngif.c,v 4.102 2006/04/29 09:13:22 tommy Exp $";

int bitmap_writepng(BITMAP* bitmap, char* filename, int interlace) {
  int i;
  FILE *fp;
  static png_structp png_ptr;
  static png_infop info_ptr;

  fp = fopen(filename, "w");
  if(!fp) {
    return(SCHEME_ERROR);
  }

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png_ptr) {
    fclose(fp);
    return(SCHEME_ERROR);
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
    return(SCHEME_ERROR);
  }

  if (setjmp(png_ptr->jmpbuf)) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return(SCHEME_ERROR);
  }

  png_init_io(png_ptr, fp);

  info_ptr->width = bitmap->w;
  info_ptr->height = bitmap->h;
  info_ptr->bit_depth = 1;
  info_ptr->interlace_type = interlace;
  info_ptr->color_type = PNG_COLOR_TYPE_GRAY;

  info_ptr->rowbytes = bitmap->rowbytes;
  info_ptr->compression_type = 0;
  png_write_info(png_ptr, info_ptr); 	/* write file header information */

  info_ptr->num_text=0;
  info_ptr->max_text=0;
  info_ptr->text=NULL;

  png_set_invert_mono(png_ptr);		/* invert pixels */

  {
    png_bytep *rows;
    rows=(png_bytep*) SchemeMalloc(bitmap->h*sizeof(png_bytep));
    for(i=0; i<bitmap->h; i++) {
      rows[i]=(png_bytep) bitmap->data+(i*(bitmap->rowbytes));
    }
    png_write_image(png_ptr, rows);
    SchemeFree(rows);
  }

  png_write_end(png_ptr, info_ptr);	/* write rest of file */

  png_destroy_write_struct(&png_ptr, &info_ptr);

  fclose(fp);
  return(SCHEME_OK);
}

int greymap_writepng(GREYMAP* greymap, char* filename, int interlace) {
  int i;
  FILE *fp;
  static png_structp png_ptr;
  static png_infop info_ptr;

  fp = fopen(filename, "w");
  if(!fp) {
    return(SCHEME_ERROR);
  }

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png_ptr) {
    fclose(fp);
    return(SCHEME_ERROR);
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
    return(SCHEME_ERROR);
  }

  if (setjmp(png_ptr->jmpbuf)) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return(SCHEME_ERROR);
  }

  png_init_io(png_ptr, fp);

  info_ptr->width = greymap->w;
  info_ptr->height = greymap->h;
  info_ptr->bit_depth = 8;
  info_ptr->interlace_type = interlace;
  info_ptr->color_type = PNG_COLOR_TYPE_GRAY;

  info_ptr->rowbytes = greymap->w;
  info_ptr->compression_type = 0;
  png_write_info(png_ptr, info_ptr); 	/* write file header information */

  info_ptr->num_text=0;
  info_ptr->max_text=0;
  info_ptr->text=NULL;

  /* png_set_invert_mono(png_ptr); */		/* invert pixels */

  {
    int i,j;
    png_bytep *rows;
    unsigned char* idata;
    idata=(unsigned char*) SchemeMalloc(greymap->w*greymap->h);
    memcpy(idata, greymap->data, greymap->w*greymap->h);
    for (j=0; j<greymap->w*greymap->h; j++)
      idata[j] = ~idata[j];

    rows=(png_bytep*) SchemeMalloc(greymap->h*sizeof(png_bytep));
    for(i=0; i<greymap->h; i++) {
      rows[i]=(png_bytep) idata+(i*(greymap->w));
    }
    png_write_image(png_ptr, rows);
    SchemeFree(rows);
    SchemeFree(idata);
  }

  png_write_end(png_ptr, info_ptr);	/* write rest of file */

  png_destroy_write_struct(&png_ptr, &info_ptr);

  fclose(fp);
  return(SCHEME_OK);
}


static int check_png(char*  file_name)
{
   FILE *fp;
   unsigned char buf[8];
   int ret;
   fp = fopen(file_name, "r");
   if (!fp)
      return 0;
   ret = fread(buf, 1, 8, fp);
   fclose(fp);
   if (ret != 8)
      return 0;
   ret = png_check_sig(buf, 8);
   return (ret);
}  

GREYMAP* greymap_readpng(char* filename) {
  GREYMAP* rc;
  FILE *fp;
  static png_structp png_ptr; 
  static png_infop info_ptr;
  int i;

  rc=(GREYMAP*) SchemeMalloc(sizeof(GREYMAP));
  if(check_png(filename)) {
    fp = fopen(filename, "r");
    if(!fp) {
      SchemeFree(rc);
      return(NULL);
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if(!png_ptr) {
      fclose(fp);
      SchemeFree(rc);
      return(NULL);
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
      fclose(fp);
      SchemeFree(rc);
      png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
      return(NULL);
    }
    if (setjmp(png_ptr->jmpbuf)) {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      fclose(fp);
      SchemeFree(rc);
      return(NULL);
    }
    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    rc->orow=-1;
    rc->ocol=-1;
    rc->xres=-1;
    rc->yres=-1;
    rc->h=info_ptr->height;
    rc->w=info_ptr->width;

    /* wenn palette, dann expandieren */
    if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_expand(png_ptr);

    /* wenn grey dann auch expandieren */
    if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
      png_set_expand(png_ptr);

    if (info_ptr->valid & PNG_INFO_tRNS)
      png_set_expand(png_ptr);

    if (info_ptr->bit_depth == 16)
      png_set_strip_16(png_ptr);

    /*
    if (info_ptr->bit_depth == 8 &&
      info_ptr->color_type == PNG_COLOR_TYPE_RGB)
      png_set_filler(png_ptr, 0x00, PNG_FILLER_BEFORE);
    */

    png_set_filler(png_ptr, 0x00, PNG_FILLER_BEFORE);

    if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY ||
      info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
         png_set_gray_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    rc->data = (unsigned char*) SchemeMalloc(rc->h*rc->w); 
    bzero(rc->data, rc->h*rc->w);

    {
      int i,r,c,v;
      png_bytep *rows;
      unsigned char* idata;

      idata=(unsigned char*) SchemeMalloc(rc->w*rc->h*4);
      rows=(png_bytep*) SchemeMalloc(rc->h*sizeof(png_bytep));

      for(i=0; i<rc->h; i++) {
	rows[i]=(png_bytep) idata+(i*(rc->w)*4);
      }
      png_read_image(png_ptr, rows);
      SchemeFree(rows);
      for(r=0; r<rc->h; r++) {
	for(c=0; c<rc->w; c++) {
	  v=((~idata[r*rc->w*4+c*4+1]&0xff)
	    +(~idata[r*rc->w*4+c*4+2]&0xff)
	    +(~idata[r*rc->w*4+c*4+3]&0xff))/3;
	  *(rc->data+r*rc->w+c)=v;
	}
      }
      SchemeFree(idata);
    }
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return(rc);

  } else {
    SchemeFree(rc);
    return(NULL);
  }
}

BITMAP* bitmap_readpng(char* filename) {
  BITMAP* rc;
  FILE *fp;
  static png_structp png_ptr;
  static png_infop info_ptr;
  int i;

  if ((rc = (BITMAP*) SchemeMalloc(sizeof(BITMAP))) == NULL) {
    fprintf(stderr,"cannot alloc BITMAP\n");
    return(NULL);
  } else {
    if(check_png(filename)) {
      fp = fopen(filename, "r");
      if(!fp) {
	SchemeFree(rc);
	return(NULL);
      }

      png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
      if(!png_ptr) {
	fclose(fp);
	SchemeFree(rc);
	return(NULL);
      }
      info_ptr = png_create_info_struct(png_ptr);
      if (!info_ptr) {
	fclose(fp);
	SchemeFree(rc);
	png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
	return(NULL);
      }
      if (setjmp(png_ptr->jmpbuf)) {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	SchemeFree(rc);
	return(NULL);
      }
      png_init_io(png_ptr, fp);
      png_read_info(png_ptr, info_ptr);

      if(info_ptr->bit_depth != 1 || info_ptr->color_type != PNG_COLOR_TYPE_GRAY) {
	fprintf(stderr,"unsupported png format for b/w bitmaps\n");
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	SchemeFree(rc);
	return(NULL);
      }

      png_set_invert_mono(png_ptr);

      rc->orow=-1;
      rc->ocol=-1;
      rc->xres=-1;
      rc->yres=-1;
      rc->h=info_ptr->height;
      rc->w=info_ptr->width;
      rc->rowbytes=(rc->w+7)/8;
      if((rc->data = (char*) SchemeMalloc(rc->h*rc->rowbytes)) == NULL) {
	fprintf(stderr,"cannot alloc bitmap data\n");
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	SchemeFree(rc);
	return(NULL);
      }
      bzero(rc->data, rc->h*rc->rowbytes);
      {
	png_bytep *rows;
	rows=(png_bytep*) SchemeMalloc(rc->h*sizeof(png_bytep));
	for(i=0; i<rc->h; i++) {
	  rows[i]=(png_bytep) rc->data+(i*(rc->rowbytes));
	}
	png_read_image(png_ptr, rows);
	SchemeFree(rows);
      }

      png_read_end(png_ptr, info_ptr);
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      fclose(fp);
      return(rc);

    } else {
      SchemeFree(rc);
      return(NULL);
    }
  }
}



