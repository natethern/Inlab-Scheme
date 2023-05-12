/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: pconvert.c,v 4.102 2006/04/29 09:13:22 tommy Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <scheme.h>
#include <bitmap.h>
#include <png.h>
#include <tiffio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <intern.h>
#include <global.h>

#define G4   COMPRESSION_CCITTFAX4
#define G3   COMPRESSION_CCITTFAX3

static char* rcsid="$Id: pconvert.c,v 4.102 2006/04/29 09:13:22 tommy Exp $";

static TIFF* t = NULL;

static char* Topen(char* file, int rows, int columns, 
      int compression,int xres, int yres, char* document, int pn, int mpn) {
  if(t != NULL) {
    return("internal error creating tiff file");
  }
  t = TIFFOpen(file, "w"); 
  if(t == NULL) {
    return("error creating tiff file");
  }
  TIFFSetField( t, TIFFTAG_IMAGEWIDTH, columns);        /* columns */
  TIFFSetField( t, TIFFTAG_IMAGELENGTH, rows);     	/* rows */
  TIFFSetField( t, TIFFTAG_BITSPERSAMPLE, 1);
  TIFFSetField( t, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
  TIFFSetField( t, TIFFTAG_COMPRESSION, compression);
  TIFFSetField( t, TIFFTAG_PHOTOMETRIC,  PHOTOMETRIC_MINISWHITE);
  TIFFSetField( t, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
  TIFFSetField( t, TIFFTAG_DOCUMENTNAME, file);
  /*
   Optional:

  TIFFSetField( t, TIFFTAG_IMAGEDESCRIPTION, "US-PTO Patent" );
  TIFFSetField( t, TIFFTAG_SOFTWARE, "patimg2tiff 1.1 by Inlab Software GmbH" );
  */
  TIFFSetField( t, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField( t, TIFFTAG_XRESOLUTION, (float) xres);
  TIFFSetField( t, TIFFTAG_YRESOLUTION, (float) yres);
  TIFFSetField( t, TIFFTAG_ROWSPERSTRIP, rows);
  TIFFSetField( t, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField( t, TIFFTAG_DOCUMENTNAME, document);
  TIFFSetField( t, TIFFTAG_PAGENUMBER, (uint16) pn, (uint16) mpn);
  return(NULL); /* OK */
}

static char* Tnewpage(char* file, int rows, int columns, 
         int compression, int xres, int yres, char* document, int pn, int mpn) {
  TIFFWriteDirectory(t);
  TIFFSetField( t, TIFFTAG_IMAGEWIDTH, columns);      /* columns */
  TIFFSetField( t, TIFFTAG_IMAGELENGTH, rows);        /* rows */
  TIFFSetField( t, TIFFTAG_BITSPERSAMPLE, 1);
  TIFFSetField( t, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
  TIFFSetField( t, TIFFTAG_COMPRESSION, compression);
  TIFFSetField( t, TIFFTAG_PHOTOMETRIC,  PHOTOMETRIC_MINISWHITE);
  TIFFSetField( t, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
  TIFFSetField( t, TIFFTAG_DOCUMENTNAME, file);
  /*
   Optional:

  TIFFSetField( t, TIFFTAG_IMAGEDESCRIPTION, "US-PTO Patent" );
  TIFFSetField( t, TIFFTAG_SOFTWARE, "patimg2tiff 1.1 by Inlab Software GmbH" );
  */
  TIFFSetField( t, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField( t, TIFFTAG_XRESOLUTION, (float) xres);
  TIFFSetField( t, TIFFTAG_YRESOLUTION, (float) yres);
  TIFFSetField( t, TIFFTAG_ROWSPERSTRIP, rows);
  TIFFSetField( t, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField( t, TIFFTAG_DOCUMENTNAME, document);
  TIFFSetField( t, TIFFTAG_PAGENUMBER, (uint16) pn, (uint16) mpn);
  return(NULL); /* OK */
}

static char* Twrite(unsigned char* buf, int length) {
  if(t == NULL) {
    return("internal error writing to tiff file");
  }
  if(TIFFWriteRawStrip(t, 0, buf, length) < 0) {
    return("error writing raw strip to tiff file");
  }
  return(NULL); /* OK */
}

static char* Tclose() {
  if(t == NULL) {
    return("internal error closing tiff file");
  }
  TIFFClose(t);
  t=NULL;
  return(NULL); /* OK */
}

/* PATIMG ************************************************************************/

extern TIFF* t;
static char *outfile="./out.tiff";

typedef struct {
  unsigned char id[8];
  unsigned char number[10];
  unsigned char pagelow[1];
  unsigned char pagehigh[1];
} HEADER1;

typedef struct {
  unsigned char offset0[1];
  unsigned char offset1[1];
  unsigned char offset2[1];
  unsigned char offset3[1];
} HEADER2;

typedef struct {
  unsigned char type0[1];
  unsigned char type1[1];
  unsigned char hres0[1];
  unsigned char hres1[1];
  unsigned char vres0[1];
  unsigned char vres1[1];
  unsigned char hpix0[1];
  unsigned char hpix1[1];
  unsigned char vpix0[1];
  unsigned char vpix1[1];
  unsigned char code0[1];
  unsigned char code1[1];
  unsigned char data[1];
} HEADER3;

static int infoflag, verboseflag;

static int getfilesize(unsigned char* file) {
  struct stat s;
  if(stat((char*) file, &s) != 0) {
    return(-1);
  }
  return(s.st_size);
}

static char* mystrdup(char *s) {
  char* rc;
  rc=(char*) SchemeMalloc(strlen(s)+1);
  strcpy(rc,s);
  return(rc);
}

static char* dopage(HEADER3* h3p, int length, int pagenumber, char* doc, int maxpage) {
  int type, hres, vres, hpix, vpix, code;
  int comp;
  char* rc;

  type=h3p->type0[0]+256*h3p->type1[0];
  hres=h3p->hres0[0]+256*h3p->hres1[0];
  vres=h3p->vres0[0]+256*h3p->vres1[0];
  hpix=h3p->hpix0[0]+256*h3p->hpix1[0];
  vpix=h3p->vpix0[0]+256*h3p->vpix1[0];
  code=h3p->code0[0]+256*h3p->code1[0];

  if(verboseflag) 
    fprintf(stderr,
	    "  type=%d, hres=%d, vres=%d, hpix=%d, vpix=%d, code=%d\n",
	    type, hres, vres, hpix, vpix, code);
    if(type==0) {
      comp=G3;
    } else if(type==1) {
      comp=G4;
    } else {
      return("invalid compression");
    }
  if(t==NULL) {
    if((rc=Topen(outfile, vpix, hpix, comp, hres, vres, doc, pagenumber, maxpage)) != NULL) {
      return(rc);
    }
  } else {
    if((rc=Tnewpage(outfile,vpix, hpix, comp, hres, vres, doc, pagenumber, maxpage)) != NULL) {
      return(rc);
    }
  }
  if((rc=Twrite(h3p->data,length-12)) != NULL) {
    return(rc);
  }
  return(NULL);
}

static char * dofile(unsigned char* file) {
  FILE* f;
  HEADER1 h1;
  HEADER2* h2p;
  HEADER3* h3p;
  int rc, count, i, filesize;
  char* number;
  char* src;
  
  filesize=getfilesize(file);
  if(filesize == -1) {
    return("cannot stat input file");
  }
  if(verboseflag) 
    fprintf(stderr,"size of %s is %d bytes\n",file, filesize);
  if(verboseflag)
    fprintf(stderr,"opening file %s\n",file);
  if((f=fopen((char*) file,"r")) == NULL) {
    return("cannot open input file");
  }
  if((rc=fread(&h1,sizeof(h1),1,f)) != 1) {
    fclose(f);
    return("cannot read patimg header");
  }
  if(strncmp((char*) h1.id, "US PTO ",8)) {
    fclose(f);
    return("input file is not a patimg file");
  }
  if(verboseflag)
    fprintf(stderr,"file contains id \"US PTO \"\n");
  {
    char tmp[11];
    strncpy(tmp, (char*) h1.number, 10);
    tmp[10]='\000';
    number=mystrdup(tmp);
  }
  if(verboseflag) 
    fprintf(stderr,"the patent number is \"%s\"\n",number);
  count=*h1.pagelow+(*h1.pagehigh*256); 
  if(verboseflag)
    fprintf(stderr,"the file contains %d pages\n", count);

  h2p=(HEADER2*) SchemeMalloc(count*sizeof(HEADER2));

  if((rc=fread(h2p,sizeof(HEADER2),count,f)) != count) {
    SchemeFree(h2p);
    SchemeFree(number);
    fclose(f);
    return("read failed reading patimg header");
  }

  for(i=0; i<count; i++) {
    int offset, offsetx, length;
    offset=h2p[i].offset0[0]+256*h2p[i].offset1[0]+
	   256*256*h2p[i].offset2[0]+256*256*256*h2p[i].offset3[0];
    if(i<count-1) {
      offsetx=h2p[i+1].offset0[0]+256*h2p[i+1].offset1[0]+
	      256*256*h2p[i+1].offset2[0]+256*256*256*h2p[i+1].offset3[0];
    } else {
      offsetx=filesize;
    }
    if(fseek(f,offset,SEEK_SET) != 0) {
      SchemeFree(h2p);
      SchemeFree(number);
      fclose(f);
      return("cannot seek to page");
    }
    length=offsetx-offset;
    if(verboseflag) {
      fprintf(stderr,"page %d offset 0x%x length %d\n",i+1,offset,length);
    }

    h3p=(HEADER3*) SchemeMalloc(length);

    if((rc=fread(h3p,length,1,f)) != 1) {
      SchemeFree(h3p);
      SchemeFree(h2p);
      SchemeFree(number);
      fclose(f);
      return("cannot read page");
    }
    if((src=dopage(h3p,length,i+1, number, count)) != NULL) {
      SchemeFree(h3p);
      SchemeFree(h2p);
      SchemeFree(number);
      fclose(f);
      return(src);
    }
    SchemeFree(h3p);
  }
  if((src=Tclose()) != NULL) {
    SchemeFree(h2p);
    SchemeFree(number);
    fclose(f);
    return(src);
  }
  SchemeFree(h2p);
  SchemeFree(number);
  fclose(f);
  return(NULL);
}

char* patimg_to_tiff(unsigned char* infile, unsigned char *tfile) {
  /*
  verboseflag=1;
  */
  outfile=(char*) tfile;
  return(dofile(infile));
}

/* ST33 **************************************************************************/

typedef struct {
  unsigned char item00[4];
  unsigned char item01[5];
  unsigned char item02[2];
  unsigned char item03[2];
  unsigned char item04[8];
  unsigned char item05[4];
  unsigned char item06[4];
  unsigned char item07[2];
  unsigned char item08[1];
  unsigned char item09[19];
  unsigned char item10[20];
  unsigned char item11[2];
  unsigned char item12[6];
  unsigned char item13[1];
  unsigned char item14[4];
  unsigned char item15[4];
  unsigned char item16[2];
  unsigned char item17[1];
  unsigned char item18[3];
  unsigned char item19[3];
  unsigned char item20[20];
  unsigned char item21[20];
  unsigned char item22[1];
  unsigned char item23[1];
  unsigned char item24[1];
  unsigned char item25[1];
  unsigned char item26[1];
  unsigned char item27[1];
  unsigned char item28[1];
  unsigned char item29[20];
  unsigned char item30[20];
  unsigned char item31[1];
  unsigned char item32[2];
  unsigned char item33[2];
  unsigned char item34[2];
  unsigned char item35[3];
  unsigned char item36[3];
  unsigned char item37[4];
  unsigned char item38[4];
  unsigned char item39[1];
  unsigned char item40[4];
  unsigned char item41[4];
  unsigned char item42[1];
  unsigned char item43[19];
  unsigned char item44[20];
  unsigned char item45[2];
  unsigned char item46[1];
} HEADER;

static char* st33dofile(int page, unsigned char* filename, int maxpage) {
  unsigned int i;
  int filesize, rc, rows, columns;
  unsigned char *buffer;
  static char *lastdocumentname=NULL;
  FILE* fp;
  HEADER* header;
  unsigned char cbuf[5];
  char documentname[32];

  filesize=getfilesize(filename);
  if(filesize == -1) {
    fprintf(stderr,"%s\n",filename);
    return("cannot stat file");
  }
  if(filesize < 256) {
    return("file to small to be reasonable");
  }

  buffer=(unsigned char*) SchemeMalloc(filesize);

  if((fp=fopen((char*) filename,"r")) == NULL) {
    return("cannot open input file");
  }

  if((rc=fread(buffer,filesize,1,fp)) != 1) {
    fclose(fp);
    SchemeFree(buffer);
    return("error reading st33 file");
  }

  header=(HEADER*) buffer;

  strncpy((char*) cbuf,(char*) header->item37,sizeof(header->item37));
  cbuf[sizeof(header->item37)]='\000';
  rows=atoi((char*) cbuf);
  
  strncpy((char*) cbuf,(char*) header->item38,sizeof(header->item38));
  cbuf[sizeof(header->item38)]='\000';
  columns=atoi((char*) cbuf);

  if(rows <= 0 || rows > 100000 || columns <= 0 || columns > 10000) {
    fclose(fp);
    SchemeFree(buffer);
    return("st33 file seems invalid");
  }

  if(verboseflag) {
    fprintf(stderr,
	    "page %d contains %d rows and %d columns.\n", page, rows, columns);
  }

  strncpy((char*) documentname,(char*) header->item02,sizeof(header->item02)),
  strncpy((char*) documentname+sizeof(header->item02),
	  (char*) header->item04,sizeof(header->item04));
  strncpy((char*) documentname+sizeof(header->item02)+sizeof(header->item04),
	  (char*) header->item03,sizeof(header->item03));
  documentname[sizeof(header->item02)+
	       sizeof(header->item04)+sizeof(header->item03)]='\000';

  for(i=0; i<sizeof(header->item02)+sizeof(header->item04)+
	     sizeof(header->item03); i++) {
    if(((documentname[i] >= 'A' && documentname[i] <= 'Z') ||
        (documentname[i] >= '0' && documentname[i] <= '9') ||
        (documentname[i] == ' '))) {
	;
     } else {
      fclose(fp);
      SchemeFree(buffer);
      return("st33 file seems invalid");
    }
  }

  if(lastdocumentname == NULL) {
    lastdocumentname=mystrdup(documentname);
  } else {
    if(strcmp(documentname, lastdocumentname)) {
      fclose(fp);
      SchemeFree(buffer);
      return("st33 file does not match to document");
    }
  }

  if(verboseflag) {
    fprintf(stderr,
      "document name in file %s is \"%s\".\n",filename,documentname);
  }

  if(t==NULL) {
    Topen(outfile, rows, columns, G4, 300, 300, documentname, page, maxpage);
  } else { 
    Tnewpage(outfile, rows, columns, G4, 300, 300, documentname, page, maxpage);
  }

  Twrite(buffer+256,filesize-256); 
  fclose(fp);
  SchemeFree(buffer);
  return(NULL);
}

char* st33_to_tiff(int nfiles, unsigned char *names, unsigned char* pfile) {
  int i,c;
  char* rc;
  unsigned char* namep;

  t=NULL;
  outfile=(char*) pfile;
  namep=names;

  for(i=0; i<nfiles; i++) {
    if(verboseflag) {
      fprintf(stderr,"processing page %04d file %s\n",i+1,namep);
    }
    rc=st33dofile(i+1,namep,nfiles);
    if(rc != NULL) {
      if(t!=NULL) {
        Tclose();
      }
      return(rc);
    }
    namep+=(strlen((char*) namep)+1);
  }
  if(t!=NULL) {
    Tclose();
  }
  return(NULL);
}

