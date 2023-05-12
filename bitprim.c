/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: bitprim.c,v 4.103 2006/04/29 09:13:22 tommy Exp $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <scheme.h>
#include <string.h>
#include <extern.h>
#include <fdecl.h>
#include <global.h>
#include <heap.h>
#include <math.h>
#include <sys/types.h>
#include <sys/times.h>

#include <bitmap.h>

char* v4lgrab();

static char* rcsid="$Id: bitprim.c,v 4.103 2006/04/29 09:13:22 tommy Exp $";

/*
 * Typedef des neuen Typen mit External am Anfang
 */

typedef struct {
  External external;
  BITMAP* bitmap;
} BMTYPE;

typedef struct {
  External external;
  GREYMAP* greymap;
} GMTYPE;

typedef struct {
  External external;
  COLORMAP* colormap;
} CMTYPE;

/*
 * Definition: wie wird der Typ ausgegeben ? Ausgabe erfolgt ueber writestring.
 */

static if_bitmap_print(Port* p, BMTYPE* e) {
  char buf[128];
  char adr[128];
  /*
  sprintf(adr,"0x%X",e->pointer);
  sprintf(buf,"#<malloc'ed %d bytes at adress %s>",e->length,(e->pointer?adr:"NULL"));
  writestring(p, buf);
  */
  if(e->bitmap != NULL) {
    /*
    rowbytes werden dem user nicht mehr gezeigt!

    sprintf(buf,"#<bitmap orow:%d ocol:%d xres:%g yres:%g h:%d w:%d b:%d>",
	  e->bitmap->orow,
	  e->bitmap->ocol,
	  e->bitmap->xres,
	  e->bitmap->yres,
	  e->bitmap->h,
	  e->bitmap->w,
	  e->bitmap->rowbytes
	  );
    */
    sprintf(buf,"#<bitmap orow:%d ocol:%d xres:%g yres:%g h:%d w:%d>",
	  e->bitmap->orow,
	  e->bitmap->ocol,
	  e->bitmap->xres,
	  e->bitmap->yres,
	  e->bitmap->h,
	  e->bitmap->w
	  );
  } else {
    sprintf(buf,"#<bitmap DESTROYED>");
  }
  writestring(p,buf);
}

static if_greymap_print(Port* p, GMTYPE* e) {
  char buf[128];
  char adr[128];
  if(e->greymap != NULL) {
    sprintf(buf,"#<greymap orow:%d ocol:%d xres:%g yres:%g h:%d w:%d>",
	  e->greymap->orow,
	  e->greymap->ocol,
	  e->greymap->xres,
	  e->greymap->yres,
	  e->greymap->h,
	  e->greymap->w
	  );
  } else {
    sprintf(buf,"#<greymap DESTROYED>");
  }
  writestring(p,buf);
}

static if_colormap_print(Port* p, CMTYPE* e) {
  char buf[128];
  char adr[128];
  if(e->colormap != NULL) {
    sprintf(buf,"#<colormap orow:%d ocol:%d xres:%g yres:%g h:%d w:%d>",
	  e->colormap->orow,
	  e->colormap->ocol,
	  e->colormap->xres,
	  e->colormap->yres,
	  e->colormap->h,
	  e->colormap->w
	  );
  } else {
    sprintf(buf,"#<colormap DESTROYED>");
  }
  writestring(p,buf);
}

/*
 * Wie wird der Typ von der GC behandelt wenn keine Referenz mehr da ist ?
 */

static if_bitmap_destroy(BMTYPE* e) {
  if(e->bitmap != NULL) {
    /*
    printf("freeing bitmap x:%d y:%d h:%d w:%d b:%d\n",
	  e->bitmap->orow,
	  e->bitmap->ocol,
	  e->bitmap->h,
	  e->bitmap->w,
	  e->bitmap->rowbytes
	  );
    */
    bitmap_destroy(e->bitmap);
  }
}

static if_greymap_destroy(GMTYPE* e) {
  if(e->greymap != NULL) {
    greymap_destroy(e->greymap);
  }
}

static if_colormap_destroy(CMTYPE* e) {
  if(e->colormap != NULL) {
    colormap_destroy(e->colormap);
  }
}

/*
 * Funktionsliste identifiziert zugleich diesen Typ.
 */

static ExternalFunctions bitmap_functions={
  if_bitmap_print,
  if_bitmap_destroy,
  NULL,			/* putchar */
  NULL,			/* putstring */
  NULL,			/* getchar */
  NULL			/* ungetchar */
};

static ExternalFunctions greymap_functions={
  if_greymap_print,
  if_greymap_destroy,
  NULL,			/* putchar */
  NULL,			/* putstring */
  NULL,			/* getchar */
  NULL			/* ungetchar */
};

static ExternalFunctions colormap_functions={
  if_colormap_print,
  if_colormap_destroy,
  NULL,			/* putchar */
  NULL,			/* putstring */
  NULL,			/* getchar */
  NULL			/* ungetchar */
};

PrimitiveProcedure(bitmap-create,2)
{
  BMTYPE* e;
  if(!IS_INTEGER(ARG(0)) || !IS_INTEGER(ARG(1))) {
    werr("argument-error, integer expected");
    return;
  } else {
    BITMAP* t;
    t=bitmap_create(C_INTEGER(ARG(1)), C_INTEGER(ARG(0))); /* Args reversed ! */
    if(t==NULL) {
      werr("error creating bitmap");
      return;
    } else {
      e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&bitmap_functions; 
      e->bitmap=t;
      val=(Pair*) e;
    }
  }
}

PrimitiveProcedure(bitmap-vccreate,2)
{
  BMTYPE* e;
  if(!IS_INTEGER(ARG(0)) || !IS_INTEGER(ARG(1))) {
    werr("argument-error, integer expected");
    return;
  } else {
    BITMAP* t;
    t=bitmap_vccreate(C_INTEGER(ARG(1)), C_INTEGER(ARG(0))); /* Args reversed ! */
    if(t==NULL) {
      werr("error creating bitmap");
      return;
    } else {
      e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&bitmap_functions; 
      e->bitmap=t;
      val=(Pair*) e;
    }
  }
}

PrimitiveProcedure(greymap-create,2)
{
  GMTYPE* e;
  if(!IS_INTEGER(ARG(0)) || !IS_INTEGER(ARG(1))) {
    werr("argument-error, integer expected");
    return;
  } else {
    GREYMAP* t;
    t=greymap_create(C_INTEGER(ARG(1)), C_INTEGER(ARG(0))); /* Args reversed ! */
    if(t==NULL) {
      werr("error creating greymap");
      return;
    } else {
      e=(GMTYPE*) AllocExternalType((sizeof(GMTYPE) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&greymap_functions; 
      e->greymap=t;
      val=(Pair*) e;
    }
  }
}

#define DEF_WIDTH   320
#define DEF_HEIGHT  240

PrimitiveProcedure(greymap-grab,0)
{
  GMTYPE* e;
  GREYMAP* t, *rc;
  char *imagedata;

  t=greymap_create(DEF_HEIGHT, DEF_WIDTH); /* Args reversed ! */
  if(t==NULL) {
    werr("error creating greymap");
    return;
  } else {
    int r,c,v;

    e=(GMTYPE*) AllocExternalType((sizeof(GMTYPE) - sizeof(External))/sizeof(SchemeWord));
    e->external.functions=&greymap_functions; 
    e->greymap=t;
    rc=t;
    val=(Pair*) e;

    imagedata=v4lgrab();
    if (imagedata != NULL) {
      for(r=0; r<DEF_HEIGHT; r++) {
        for(c=0; c<DEF_WIDTH; c++) {
            v=((~imagedata[r*rc->w*3+c*3+1]&0xff)
              +(~imagedata[r*rc->w*3+c*3+2]&0xff)
              +(~imagedata[r*rc->w*3+c*3+3]&0xff))/3;
            *(rc->data+r*rc->w+c)=v;
        }
      }                              
    }

  }
}

PrimitiveProcedure(colormap-create,2)
{
  CMTYPE* e;
  if(!IS_INTEGER(ARG(0)) || !IS_INTEGER(ARG(1))) {
    werr("argument-error, integer expected");
    return;
  } else {
    COLORMAP* t;
    t=colormap_create(C_INTEGER(ARG(1)), C_INTEGER(ARG(0))); /* Args reversed ! */
    if(t==NULL) {
      werr("error creating colormap");
      return;
    } else {
      e=(CMTYPE*) AllocExternalType((sizeof(CMTYPE) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&greymap_functions; 
      e->colormap=t;
      val=(Pair*) e;
    }
  }
}

PrimitiveProcedure(bitmap-readxbm,1)
{
  BMTYPE* e;
  if(!IS_STRING(ARG(0))) {
    werr("argument-error, string expected");
    return;
  } else {
    BITMAP* t;
    t=bitmap_readxbm((char*) C_STRING(ARG(0)));
    if(t==NULL) {
      werr("error reading XBM-file");
      return;
    } else {
      e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&bitmap_functions; 
      e->bitmap=t;
      val=(Pair*) e;
    }
  }
}

PrimitiveProcedure(bitmap-readtiff,1)
{
  BMTYPE* e;
  if(!IS_STRING(ARG(0))) {
    werr("argument-error, string expected");
    return;
  } else {
    BITMAP* t;
    t=bitmap_readtiff((char*) C_STRING(ARG(0)), 0);
    if(t==NULL) {
      werr("error reading TIFF-file");
      return;
    } else {
      e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&bitmap_functions; 
      e->bitmap=t;
      val=(Pair*) e;
    }
  }
}

PrimitiveProcedure(bitmap-readpng,1)
{
  BMTYPE* e;
  if(!IS_STRING(ARG(0))) {
    werr("argument-error, string expected");
    return;
  } else {
    BITMAP* t;
    t=bitmap_readpng((char*) C_STRING(ARG(0)));
    if(t==NULL) {
      werr("error reading PNG file");
      return;
    } else {
      e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&bitmap_functions; 
      e->bitmap=t;
      val=(Pair*) e;
    }
  }
}


PrimitiveProcedure(greymap-readpng,1)
{
  GMTYPE* e;
  if(!IS_STRING(ARG(0))) {
    werr("argument-error, string expected");
    return;
  } else {
    GREYMAP* t;
    t=greymap_readpng((char*) C_STRING(ARG(0)));
    if(t==NULL) {
      werr("error reading PNG file");
      return;
    } else {
      e=(GMTYPE*) AllocExternalType((sizeof(GMTYPE) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&greymap_functions; 
      e->greymap=t;
      val=(Pair*) e;
    }
  }
}


PrimitiveProcedure(bitmap-tiffpages,1)
{
  BMTYPE* e;
  if(!IS_STRING(ARG(0))) {
    werr("argument-error, string expected");
    return;
  } else {
    int t;
    t=bitmap_npages((char*) C_STRING(ARG(0)));
    if(t==SCHEME_ERROR) {
      werr("error opening TIFF-file");
      return;
    } else {
      val=AllocInteger();
      val->car=(Pair*) t;
    }
  }
}

PrimitiveProcedure(bitmap-readmtiff,2)
{
  BMTYPE* e;
  if(!IS_INTEGER(ARG(0)) ||!IS_STRING(ARG(1))) {
    werr("argument error");
    return;
  } else {
    BITMAP* t;
    t=bitmap_readtiff((char*) C_STRING(ARG(1)), C_INTEGER(ARG(0)));
    if(t==NULL) {
      val=false_constant;
      return;
    } else {
      e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&bitmap_functions; 
      e->bitmap=t;
      val=(Pair*) e;
    }
  }
}

PrimitiveProcedure(bitmap-width,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      val=AllocInteger();
      val->car=(Pair*) e->bitmap->w;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(greymap-width,1)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      val=AllocInteger();
      val->car=(Pair*) e->greymap->w;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(greymap-height,1)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      val=AllocInteger();
      val->car=(Pair*) e->greymap->h;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(bitmap-height,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      val=AllocInteger();
      val->car=(Pair*) e->bitmap->h;
    } else {
      val=false_constant;
    }
  }
}

/*
  PrimitiveProcedure(bitmap-destroy!,1)
  {
    BMTYPE* e;
    e=(BMTYPE*) ARG(0);
    if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
      werr("argument-error");
      return;
    } else {
      bitmap_destroy(e->bitmap);
      e->bitmap = NULL;
      val=ok_atom;
    }
  }
*/

PrimitiveProcedure(bitmap-orow,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      val=AllocInteger();
      val->car=(Pair*) e->bitmap->orow;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(greymap-orow,1)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      val=AllocInteger();
      val->car=(Pair*) e->greymap->orow;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(bitmap-orow-set!,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions || !IS_INTEGER(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      e->bitmap->orow=C_INTEGER(ARG(0));
    } else {
      werr("bitmap-orow-set!: bitmap destroyed!");
      return;
    }
  }
}

PrimitiveProcedure(greymap-orow-set!,2)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions || !IS_INTEGER(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      e->greymap->orow=C_INTEGER(ARG(0));
    } else {
      werr("greymap-orow-set!: bitmap destroyed!");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-ocol-set!,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions || !IS_INTEGER(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      e->bitmap->ocol=C_INTEGER(ARG(0));
    } else {
      werr("bitmap-ocol-set!: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-ocol-set!,2)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions || !IS_INTEGER(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      e->greymap->ocol=C_INTEGER(ARG(0));
    } else {
      werr("greymap-ocol-set!: bitmap destroyed");
    }
  }
}

PrimitiveProcedure(greymap-ocol,1)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      val=AllocInteger();
      val->car=(Pair*) e->greymap->ocol;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(bitmap-ocol,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      val=AllocInteger();
      val->car=(Pair*) e->bitmap->ocol;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(bitmap-rowbytes,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      val=AllocInteger();
      val->car=(Pair*) e->bitmap->rowbytes;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(bitmap-writetiff,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_STRING(ARG(0))) {
        werr("argument-error, string expected");
        return;
      } else { 
        int rc; 
        rc=bitmap_writetiff(e->bitmap, (char*) C_STRING(ARG(0)));
        if(rc != SCHEME_OK) {
          werr("error writing TIFF-file");
          return;
        } else {
          val=ok_atom;
        }
      }
    } else {
      werr("bitmap-writetiff: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-writexbm,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_STRING(ARG(0))) {
        werr("argument-error, string expected");
        return;
      } else { 
        int rc; 
        rc=bitmap_writexbm(e->bitmap, (char*) C_STRING(ARG(0)));
        if(rc != SCHEME_OK) {
          werr("error writing XBM");
          return;
        } else {
          val=ok_atom;
        }
      }
    } else {
      werr("bitmap-writexbm: bitmap destroyed");
      return;
    }
  }
}

/*

 PrimitiveProcedure(bitmap-writegif,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_STRING(ARG(0))) {
        werr("argument-error, string expected");
        return;
      } else { 
        int rc; 
        rc=bitmap_writegif(e->bitmap, (char*) C_STRING(ARG(0)));
        if(rc != SCHEME_OK) {
          werr("error writing GIF");
          return;
        } else {
          val=ok_atom;
        }
      }
    } else {
      werr("bitmap-writegif: bitmap destroyed");
      return;
    }
  }
}

 PrimitiveProcedure(bitmap-writeigif,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_STRING(ARG(0))) {
        werr("argument-error, string expected");
        return;
      } else { 
        int rc; 
        rc=bitmap_writeigif(e->bitmap, (char*) C_STRING(ARG(0)));
        if(rc != SCHEME_OK) {
          werr("error writing GIF");
          return;
        } else {
          val=ok_atom;
        }
      }
    } else {
      werr("bitmap-writeigif: bitmap destroyed");
      return;
    }
  }
}

 PrimitiveProcedure(bitmap-writehalfgif,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_STRING(ARG(0))) {
        werr("argument-error, string expected");
        return;
      } else { 
        int rc; 
        rc=bitmap_writehalfgif(e->bitmap, (char*) C_STRING(ARG(0)));
        if(rc != SCHEME_OK) {
          werr("error writing GIF");
          return;
        } else {
          val=ok_atom;
        }
      }
    } else {
      werr("bitmap-writhalfgif: bitmap destroyed");
      return;
    }
  }
}

 PrimitiveProcedure(bitmap-writehalfigif,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_STRING(ARG(0))) {
        werr("argument-error, string expected");
        return;
      } else { 
        int rc; 
        rc=bitmap_writehalfigif(e->bitmap, (char*) C_STRING(ARG(0)));
        if(rc != SCHEME_OK) {
          werr("error writing GIF");
          return;
        } else {
          val=ok_atom;
        }
      }
    } else {
      werr("bitmap-writhalfigif: bitmap destroyed");
      return;
    }
  }
}

*/

PrimitiveProcedure(bitmap-appendtiff,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_STRING(ARG(0))) {
        werr("argument-error, string expected");
        return;
      } else { 
        int rc; 
        rc=bitmap_appendtiff(e->bitmap, (char*) C_STRING(ARG(0)));
        if(rc != SCHEME_OK) {
          werr("error appending to TIFF-file");
          return;
        } else {
          val=ok_atom;
        }
      }
    } else {
      werr("bitmap-appendtiff: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-invert!,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      bitmap_invert(e->bitmap);
      val=ARG(0);
    } else {
      werr("bitmap-invert!: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-invert!,1)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      greymap_invert(e->greymap);
      val=ARG(0);
    } else {
      werr("greymap-invert!: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-copy,1)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      GREYMAP* t;
      t=greymap_copy(e->greymap);
      if(t==NULL) {
        werr("error copying bitmap");
        return;
      } else {
        e=(GMTYPE*) AllocExternalType((sizeof(GMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&greymap_functions; 
        e->greymap=t;
        val=(Pair*) e;
      }
    } else {
      werr("greymap-copy: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-copy,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_copy(e->bitmap);
      if(t==NULL) {
        werr("error copying bitmap");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-copy: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-rowdiff,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_rowdiff(e->bitmap);
      if(t==NULL) {
        werr("error bitmap-rowdiff");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-rowdiff: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-rowdiff-reverse,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_rowdiff_reverse(e->bitmap);
      if(t==NULL) {
        werr("error bitmap-rowdiff-reverse");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-rowdiff-reverse: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap->bitmap,1)
{
  GMTYPE* e;
  BMTYPE* g;
  e=(GMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      BITMAP* t;
      t=greymap_to_bitmap(e->greymap,128);
      if(t==NULL) {
        werr("error creating bitmap");
        return;
      } else {
        g=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        g->external.functions=&bitmap_functions; 
        g->bitmap=t;
        val=(Pair*) g;
      }
    } else {
      werr("greymap->bitmap: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap->greymap,1)
{
  GMTYPE* e;
  BMTYPE* g;
  g=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || g->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(g->bitmap != NULL) {
      GREYMAP* t;
      t=bitmap_to_greymap(g->bitmap);
      if(t==NULL) {
        werr("error creating greymap");
        return;
      } else {
        e=(GMTYPE*) AllocExternalType((sizeof(GMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&greymap_functions; 
        e->greymap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap->greymap: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap->bitmap/t,2)
{
  GMTYPE* e;
  BMTYPE* g;
  e=(GMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions || !IS_INTEGER(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    int t;
    t=C_INTEGER(ARG(0));
    if(t<0 || t>255) {
      werr("invalid threshold");
      return;
    } else {
      if(e->greymap != NULL) {
	BITMAP* t;
	t=greymap_to_bitmap(e->greymap,C_INTEGER(ARG(0)));
	if(t==NULL) {
	  werr("error creating bitmap");
	  return;
	} else {
	  g=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
	  g->external.functions=&bitmap_functions; 
	  g->bitmap=t;
	  val=(Pair*) g;
	}
      } else {
        werr("greymap->bitmap: bitmap destroyed");
	return;
      }
    }
  }
}

PrimitiveProcedure(bitmap-scale,3)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(2);
  if(ARG(2)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions ||
     !IS_FLOAT(ARG(1)) || !IS_FLOAT(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_scale(e->bitmap,C_FLOAT(ARG(0)),C_FLOAT(ARG(1)));
      if(t==NULL) {
        werr("error copying bitmap");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-scale: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-scale,3)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(2);
  if(ARG(2)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions ||
     !IS_FLOAT(ARG(1)) || !IS_FLOAT(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      GREYMAP* t;
      t=greymap_scale(e->greymap,C_FLOAT(ARG(0)),C_FLOAT(ARG(1)));
      if(t==NULL) {
        werr("error scaling greymap");
        return;
      } else {
        e=(GMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&greymap_functions; 
        e->greymap=t;
        val=(Pair*) e;
      }
    } else {
      werr("greymap-scale: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-scale-absolute,3)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(2);
  if(ARG(2)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions ||
     !IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(0)) || C_INTEGER(ARG(0)) < 1 || C_INTEGER(ARG(1)) < 1) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_scaleabsolute(e->bitmap,C_INTEGER(ARG(0)),C_INTEGER(ARG(1)));
      if(t==NULL) {
        werr("error copying bitmap");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-scale-absolute: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-rotate-left,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_lrotate(e->bitmap);
      if(t==NULL) {
        werr("error rotating bitmap");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-rotate-right: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-rotate-right,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_rrotate(e->bitmap);
      if(t==NULL) {
        werr("error rotating bitmap");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-rotate-right: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-getpixel,3)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(2);
  if(ARG(2)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(0))) {
        werr("argument-error");
	return;
      } else {
        int rc;
        if((rc=bitmap_getpixel(e->bitmap, C_INTEGER(ARG(1)), C_INTEGER(ARG(0)))) == 1) {
	  val=true_constant;
        } else {
	  if(rc == SCHEME_ERROR) {
            werr("dimension out of range");
	    return;
	  } else {
	    val=false_constant;
	  }
        }
      }
    } else {
      werr("bitmap-getpixel: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-getpixel,3)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(2);
  if(ARG(2)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      if(!IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(0))) {
        werr("argument-error");
	return;
      } else {
        int rc;
        rc=greymap_getpixel(e->greymap, C_INTEGER(ARG(1)), C_INTEGER(ARG(0)));
	if(rc == SCHEME_ERROR) {
	  werr("dimension out of range");
	  return;
	} else {
	  MakeInteger(val,rc);
	}
      }
    } else {
      werr("greymap-getpixel: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-setpixel!,3)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(2);
  if(ARG(2)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(0))) {
        werr("argument-error");
	return;
      } else {
        int rc;
        if((rc=bitmap_setpixel(e->bitmap, C_INTEGER(ARG(1)), C_INTEGER(ARG(0)))) == SCHEME_ERROR) {
	  werr("error (dimension?)");
	  return;
        } else {
	  val=ARG(2);
        }
      }
    } else {
      werr("bitmap-setpixel!: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-setpixel!,4)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(3);
  if(ARG(3)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      if(!IS_INTEGER(ARG(2)) || !IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(0))) {
        werr("argument-error");
	return;
      } else {
        int rc;
	if(C_INTEGER(ARG(0)) < 0 || C_INTEGER(ARG(0)) > 255) {
          werr("grey value out of range");
	  return;
	} else {
          if((rc=greymap_setpixel(e->greymap, C_INTEGER(ARG(2)), C_INTEGER(ARG(1)),
				  C_INTEGER(ARG(0)))) == SCHEME_ERROR) {
	    werr("error (dimension?)");
	    return;
          } else {
	    val=ARG(3);
          }
        }
      }
    } else {
      werr("greymap-setpixel!: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-clearpixel!,3)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(2);
  if(ARG(2)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(0))) {
        werr("argument-error");
	return;
      } else {
        int rc;
        if((rc=bitmap_clearpixel(e->bitmap, C_INTEGER(ARG(1)), C_INTEGER(ARG(0)))) == SCHEME_ERROR) {
	  werr("error (dimension?)");
	  return;
        } else {
	  val=ARG(2);
        }
      }
    } else {
      werr("bitmap-clearpixel!: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-half,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_half(e->bitmap);
      if(t==NULL) {
        werr("error scaling bitmap");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-half: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-8,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_8(e->bitmap);
      if(t==NULL) {
        werr("error scaling bitmap");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-8: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-half4,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_half4(e->bitmap);
      if(t==NULL) {
        werr("error scaling bitmap");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-half: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-vcdouble,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_vcdouble(e->bitmap);
      if(t==NULL) {
        werr("error scaling bitmap");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-double: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-double,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      BITMAP* t;
      t=bitmap_double(e->bitmap);
      if(t==NULL) {
        werr("error scaling bitmap");
        return;
      } else {
        e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        e->external.functions=&bitmap_functions; 
        e->bitmap=t;
        val=(Pair*) e;
      }
    } else {
      werr("bitmap-double: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-compare,2)
{
  BMTYPE*e,*f;
  e=(BMTYPE*) ARG(0);
  f=(BMTYPE*) ARG(1);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions ||
     ARG(1)->tag != EXTERNAL_TYPE || f->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL && f->bitmap != NULL) {
      int epix,tpix;
      if(bitmap_compare(e->bitmap, f->bitmap, &epix, &tpix) != SCHEME_OK) {
        werr("error comparing bitmaps");
        return;
      } else {
        SetPairIn(val);
        val->car=false_constant;
        val->cdr=false_constant;
        val->car=AllocInteger();
        val->car->car=(Pair*) epix;
        val->cdr=AllocInteger();
        val->cdr->car=(Pair*) tpix;
      }
    } else {
      werr("bitmap-compare: at least one bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-implant!,2)
{
  BMTYPE*e,*f;
  e=(BMTYPE*) ARG(0);
  f=(BMTYPE*) ARG(1);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions ||
     ARG(1)->tag != EXTERNAL_TYPE || f->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL && f->bitmap != NULL) {
      if(bitmap_implant(f->bitmap, e->bitmap) != SCHEME_OK) {
        werr("error implanting! bitmap");
	return;
      } else { 
        val=ARG(0);
      }
    } else {
      werr("bitmap-implant!: at least one bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-implantxor!,2)
{
  BMTYPE*e,*f;
  e=(BMTYPE*) ARG(0);
  f=(BMTYPE*) ARG(1);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions ||
     ARG(1)->tag != EXTERNAL_TYPE || f->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL && f->bitmap != NULL) {
      if(bitmap_implantxor(f->bitmap, e->bitmap) != SCHEME_OK) {
        werr("error implanting! bitmap");
	return;
      } else { 
        val=ARG(0);
      }
    } else {
      werr("bitmap-implant!: at least one bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-nextblack,3)
{
  BMTYPE*e;
  e=(BMTYPE*) ARG(2);
  if(ARG(2)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions ||
     !IS_INTEGER(ARG(0)) || !IS_INTEGER(ARG(1))) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      int r = -1;
      int c = -1;
      if(bitmap_nextblack(e->bitmap, C_INTEGER(ARG(1)), C_INTEGER(ARG(0)), &r, &c) != SCHEME_OK) {
        werr("error bitmap-nextblack");
        return;
      } else {
        SetPairIn(val);
        val->car=false_constant;
        val->cdr=false_constant;
        val->car=AllocInteger();
        val->car->car=(Pair*) r;
        val->cdr=AllocInteger();
        val->cdr->car=(Pair*) c;
      }
    } else {
      werr("bitmap-nextblack: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-nextblackup,3)
{
  BMTYPE*e;
  e=(BMTYPE*) ARG(2);
  if(ARG(2)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions ||
     !IS_INTEGER(ARG(0)) || !IS_INTEGER(ARG(1))) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      int r = -1;
      int c = -1;
      if(bitmap_nextblackup(e->bitmap, C_INTEGER(ARG(1)), C_INTEGER(ARG(0)), &r, &c) != SCHEME_OK) {
        werr("error bitmap-nextblackup");
        return;
      } else {
        SetPairIn(val);
        val->car=false_constant;
        val->cdr=false_constant;
        val->car=AllocInteger();
        val->car->car=(Pair*) r;
        val->cdr=AllocInteger();
        val->cdr->car=(Pair*) c;
      }
    } else {
      werr("bitmap-nextblackup: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-extract!,3)
{
  BMTYPE *f, *e;
  f=(BMTYPE*) ARG(2);
  if(ARG(2)->tag != EXTERNAL_TYPE || f->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(!IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(0))) {
      werr("argument-error");
      return;
    } else {
      if(f->bitmap != NULL) {
        BITMAP* t;
        t=bitmap_extract(f->bitmap, C_INTEGER(ARG(1)), C_INTEGER(ARG(0)));
        if(t==NULL) {
	  val=false_constant;
	  return;
        } else {
	  e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
	  e->external.functions=&bitmap_functions; 
	  e->bitmap=t;
	  val=(Pair*) e;
        }
      } else {
        werr("bitmap-extract: bitmap destroyed");
	return;
      }
    }
  }
}

PrimitiveProcedure(bitmap->string,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      val=make_string(e->bitmap->h*e->bitmap->rowbytes,'\000');
      memcpy(C_STRING(val), e->bitmap->data, e->bitmap->h*e->bitmap->rowbytes); 
    } else {
      werr("bitmap->string: bitmap destroyed");
      val=false_constant;
      return;
    }
  }
}


PrimitiveProcedure(string->bitmap,3)
{
  BMTYPE* e;
  if(!IS_INTEGER(ARG(0)) || !IS_INTEGER(ARG(1)) || !IS_STRING(ARG(2))) {
    werr("argument-error");
    return;
  } else {
    BITMAP* t;
    if(C_STRINGLEN(ARG(2)) != C_INTEGER(ARG(1))*((C_INTEGER(ARG(0))+7)/8)) {
      werr("dimension / stringlen-mismatch");
      val=false_constant;
      return;
    }
    t=bitmap_create(C_INTEGER(ARG(1)), C_INTEGER(ARG(0))); /* Args reversed ! */
    if(t==NULL) {
      werr("error creating bitmap");
      return;
    } else {
      e=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&bitmap_functions; 
      e->bitmap=t;
      memcpy(e->bitmap->data, C_STRING(ARG(2)), e->bitmap->h*e->bitmap->rowbytes);
      val=(Pair*) e;
    }
  }
}

PrimitiveProcedure(bitmap-grey!,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      bitmap_grey(e->bitmap);
      val=ARG(0);
    } else {
      werr("bitmap-grey!: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-hstripe!,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      bitmap_hstripe(e->bitmap);
      val=ARG(0);
    } else {
      werr("bitmap-hstripe!: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-vstripe!,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      bitmap_vstripe(e->bitmap);
      val=ARG(0);
    } else {
      werr("bitmap-vstripe!: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap?,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    val=false_constant;
  } else {
    val=true_constant;
  }
}

PrimitiveProcedure(greymap?,1)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    val=false_constant;
  } else {
    val=true_constant;
  }
}

PrimitiveProcedure(bitmap-xres,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      val=AllocFloat();
      C_FLOAT(val)=e->bitmap->xres;
    } else {
      werr("bitmap-xres: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-xres,1)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      val=AllocFloat();
      C_FLOAT(val)=e->greymap->xres;
    } else {
      werr("greymap-xres: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-yres,1)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      val=AllocFloat();
      C_FLOAT(val)=e->greymap->yres;
    } else {
      werr("greymap-yres: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-yres,1)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      val=AllocFloat();
      C_FLOAT(val)=e->bitmap->yres;
    } else {
      werr("bitmap-yres: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-xres-set!,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || 
     e->external.functions != &bitmap_functions || !IS_FLOAT(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      e->bitmap->xres=C_FLOAT(ARG(0));
      val=ARG(0);
    } else {
      werr("bitmap-xres-set!: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-xres-set!,2)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || 
     e->external.functions != &greymap_functions || !IS_FLOAT(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      e->greymap->xres=C_FLOAT(ARG(0));
      val=ARG(0);
    } else {
      werr("greymap-xres-set!: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-yres-set!,2)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || 
     e->external.functions != &greymap_functions || !IS_FLOAT(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      e->greymap->yres=C_FLOAT(ARG(0));
      val=ARG(0);
    } else {
      werr("greymap-xres-set!: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-yres-set!,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || 
     e->external.functions != &bitmap_functions || !IS_FLOAT(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      e->bitmap->yres=C_FLOAT(ARG(0));
    } else {
      werr("bitmap-yres-set!: bitmap destroyed");
      return;
    }
    val=ARG(0);
  }
}

PrimitiveProcedure(bitmap-equal?,2)
{
  BMTYPE*e,*f;
  e=(BMTYPE*) ARG(0);
  f=(BMTYPE*) ARG(1);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions ||
     ARG(1)->tag != EXTERNAL_TYPE || f->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL && f->bitmap != NULL) {
      if(bitmap_equal(e->bitmap, f->bitmap) == SCHEME_TRUE) {
        val=true_constant;
      } else {
        val=false_constant;
      }
    } else {
      werr("bitmap-equal?: at least one bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-equal?,2)
{
  GMTYPE*e,*f;
  e=(GMTYPE*) ARG(0);
  f=(GMTYPE*) ARG(1);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions ||
     ARG(1)->tag != EXTERNAL_TYPE || f->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL && f->greymap != NULL) {
      if(greymap_equal(e->greymap, f->greymap) == SCHEME_TRUE) {
        val=true_constant;
      } else {
        val=false_constant;
      }
    } else {
      werr("greymap-equal?: at least one bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-hash,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || 
     e->external.functions != &bitmap_functions || !IS_INTEGER(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      int i;
      i=bitmap_fold(e->bitmap, C_INTEGER(ARG(0)));
      val=AllocInteger();
      val->car=(Pair*) i;
    } else {
      werr("bitmap-hash: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-drawline!,5)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(4);
  if(ARG(4)->tag != EXTERNAL_TYPE || 
     e->external.functions != &bitmap_functions || !IS_INTEGER(ARG(0)) 
     || !IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(2)) || !IS_INTEGER(ARG(3))) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      int i;
      i=bitmap_drawline(e->bitmap, C_INTEGER(ARG(3)), C_INTEGER(ARG(2)), C_INTEGER(ARG(1)), C_INTEGER(ARG(0)));
      if(i!=SCHEME_OK) {
        werr("arguments aut of range");
	return;
      } else {
	val=ok_atom;
      }
    } else {
      werr("bitmap-hash: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-writepng,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_STRING(ARG(0))) {
        werr("argument-error, string expected");
        return;
      } else { 
        int rc; 
        rc=bitmap_writepng(e->bitmap, (char*) C_STRING(ARG(0)), 0);
        if(rc != SCHEME_OK) {
          werr("error writing PNG");
          return;
        } else {
          val=ok_atom;
        }
      }
    } else {
      werr("bitmap-writepng: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-writeipng,2)
{
  BMTYPE* e;
  e=(BMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      if(!IS_STRING(ARG(0))) {
        werr("argument-error, string expected");
        return;
      } else { 
        int rc; 
        rc=bitmap_writepng(e->bitmap, (char*) C_STRING(ARG(0)), 1);
        if(rc != SCHEME_OK) {
          werr("error writing PNG");
          return;
        } else {
          val=ok_atom;
        }
      }
    } else {
      werr("bitmap-writepng: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-writepng,2)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      if(!IS_STRING(ARG(0))) {
        werr("argument-error, string expected");
        return;
      } else { 
        int rc; 
        rc=greymap_writepng(e->greymap, (char*) C_STRING(ARG(0)), 0);
        if(rc != SCHEME_OK) {
          werr("error writing PNG");
          return;
        } else {
          val=ok_atom;
        }
      }
    } else {
      werr("greymap-writepng: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-writeipng,2)
{
  GMTYPE* e;
  e=(GMTYPE*) ARG(1);
  if(ARG(1)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      if(!IS_STRING(ARG(0))) {
        werr("argument-error, string expected");
        return;
      } else { 
        int rc; 
        rc=greymap_writepng(e->greymap, (char*) C_STRING(ARG(0)), 1);
        if(rc != SCHEME_OK) {
          werr("error writing PNG");
          return;
        } else {
          val=ok_atom;
        }
      }
    } else {
      werr("greymap-writepng: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(bitmap-crop,5)
{
  BMTYPE*e, *f;
  BITMAP* t;
  e=(BMTYPE*) ARG(4);
  if(ARG(4)->tag != EXTERNAL_TYPE || e->external.functions != &bitmap_functions ||
     !IS_INTEGER(ARG(0)) || !IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(2)) || !IS_INTEGER(ARG(3))) {
    werr("argument-error");
    return;
  } else {
    if(e->bitmap != NULL) {
      t=bitmap_crop(e->bitmap, 
		    C_INTEGER(ARG(3)), C_INTEGER(ARG(2)), C_INTEGER(ARG(1)), C_INTEGER(ARG(0)));
      if(t==NULL) {
        werr("error cropping bitmap, dimensions out of range");
	return;
      } else { 
        f=(BMTYPE*) AllocExternalType((sizeof(BMTYPE) - sizeof(External))/sizeof(SchemeWord));
        f->external.functions=&bitmap_functions; 
        f->bitmap=t;
        val=(Pair*) f;
      }
    } else {
      werr("bitmap-crop: bitmap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-crop,5)
{
  GMTYPE*e, *f;
  GREYMAP* t;
  e=(GMTYPE*) ARG(4);
  if(ARG(4)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions ||
     !IS_INTEGER(ARG(0)) || !IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(2)) || !IS_INTEGER(ARG(3))) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      t=greymap_crop(e->greymap, 
		    C_INTEGER(ARG(3)), C_INTEGER(ARG(2)), C_INTEGER(ARG(1)), C_INTEGER(ARG(0)));
      if(t==NULL) {
        werr("error cropping greymap, dimensions out of range");
	return;
      } else { 
        f=(GMTYPE*) AllocExternalType((sizeof(GMTYPE) - sizeof(External))/sizeof(SchemeWord));
        f->external.functions=&greymap_functions; 
        f->greymap=t;
        val=(Pair*) f;
      }
    } else {
      werr("greymap-crop: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(greymap-smooth-region,6)
{
  GMTYPE*e, *f;
  GREYMAP* t;
  e=(GMTYPE*) ARG(5);
  if(ARG(5)->tag != EXTERNAL_TYPE || e->external.functions != &greymap_functions ||
     !IS_INTEGER(ARG(0)) || !IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(2)) || !IS_INTEGER(ARG(3)) ||
     !IS_INTEGER(ARG(4))) {
    werr("argument-error");
    return;
  } else {
    if(e->greymap != NULL) {
      t=greymap_smooth_region(e->greymap, 
		    C_INTEGER(ARG(4)), C_INTEGER(ARG(3)), C_INTEGER(ARG(2)), C_INTEGER(ARG(1)),
		    C_INTEGER(ARG(0)));
      if(t==NULL) {
        werr("error smoothing greymap, dimensions out of range");
	return;
      } else { 
        f=(GMTYPE*) AllocExternalType((sizeof(GMTYPE) - sizeof(External))/sizeof(SchemeWord));
        f->external.functions=&greymap_functions; 
        f->greymap=t;
        val=(Pair*) f;
      }
    } else {
      werr("greymap-smooth-region: greymap destroyed");
      return;
    }
  }
}

PrimitiveProcedure(patimg-to-tiff,2) 
{
  char* rc;
  if(!IS_STRING(ARG(0)) || !IS_STRING(ARG(1))) {
    werr("argument error");
    return;
  } else {
    rc=patimg_to_tiff(C_STRING(ARG(1)), C_STRING(ARG(0)));
    if(rc==NULL) {
      val=ok_atom;
    } else {
      werr(rc);
      return;
    }
  }
}

PrimitiveProcedure(internal-st33-to-tiff,3) 
{
  char* rc;
  if(!IS_STRING(ARG(0)) || !IS_STRING(ARG(1)) || !IS_INTEGER(ARG(2))) {
    werr("argument error");
    return;
  } else {
    rc=st33_to_tiff(C_INTEGER(ARG(2)), C_STRING(ARG(1)), C_STRING(ARG(0)));
    if(rc==NULL) {
      val=ok_atom;
    } else {
      werr(rc);
      return;
    }
  }
}

