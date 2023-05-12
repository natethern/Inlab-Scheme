/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: auxmem.c,v 4.102 2006/04/29 09:13:22 tommy Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <scheme.h>
#include <fdecl.h>
#include <extern.h>
#include <global.h>

#define PATTERN '\123' 

static char *rcsid = "$Id: auxmem.c,v 4.102 2006/04/29 09:13:22 tommy Exp $";

static int SchemeAuxmem=0;
extern int SchemeAuxmemMax;

char* SchemeMalloc(int size) {
  char* t;

  if(SchemeAuxmem+size > SchemeAuxmemMax) { 
    garbage_collect();
  }
  /*
  if(SchemeAuxmem+size > SchemeAuxmemMax) {
    SchemePanic("out of auxiliary memory, try to increase SCHEMEAUXMEM"); 
  }
  */

  t=(char*) malloc(size + sizeof(SchemeWord) + 1);

  if(t != NULL) {
    *((SchemeWord*) t)=size;
    *(t+sizeof(SchemeWord)+size)=PATTERN;
    SchemeAuxmem+=size;
    return(t+sizeof(SchemeWord));
  } else {
    SchemePanic("out of memory");
    exit(2);
  }
}

void SchemeFree(char* p) {
  char *t;
  SchemeAuxmem-=*((SchemeWord*) (p-sizeof(SchemeWord))); 

  t=p-sizeof(SchemeWord);

  if(*((SchemeWord*) t) == 0) {
    SchemePanic("allocation error 1");
  }

  if(*(t+sizeof(SchemeWord)+*((SchemeWord*) t)) != PATTERN) {
    fprintf(stderr,"*** SIZE == %d ****\n",*((SchemeWord*) t));
    SchemePanic("allocation error 2");
  }
  *((SchemeWord*) t)=0;
  *(t+sizeof(SchemeWord)+*((SchemeWord*) t))='\000';
  free(t);
}

int SchemeGetAuxmem() {
  return(SchemeAuxmem);
}

int SchemeGetAuxmemMax() {
  return(SchemeAuxmemMax);
}

void SchemeSetAuxmemMax(int v) {
  SchemeAuxmemMax=v;
}

