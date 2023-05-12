/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: scheme.c,v 4.102 2006/04/29 09:13:22 tommy Exp $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <scheme.h>
#include <fdecl.h>
#include <intern.h>
#include <global.h>

static char *rcsid = "$Id: scheme.c,v 4.102 2006/04/29 09:13:22 tommy Exp $";

/*                  	Min	Default
 *  SCHEME_PSTACKSIZE 	4096	16K
 *  SCHEME_CSTACKSIZE	4096	16K
 *  SCHEME_HASHSIZE	4096	4096
 *  SCHEME_HEAPSIZE	2M	4M
 *  SCHEME_AUXMEMSIZE	1M	16M
 */

static int askenv(char* variable, int minimal, int def) {
  char *s;
  int value;
  if((s=getenv(variable)) == NULL) {
    return(def);
  } else {
    unsigned int i;
    for(i=0; i<strlen(s); i++) {
      if(s[i] < '0' || s[i] > '9') {
	if(s[i] != 'M' && s[i] != 'K') {
          fprintf(stderr,
	    "WARNING: invalid %s found, using default.\n",variable);
          return(def);
	} else {
	  if(i<(strlen(s)-1)) {
	    fprintf(stderr,
	      "WARNING: invalid %s found, using default.\n",variable);
	    return(def);
	  }
	}
      }
    }
    value=atoi(s);
    if(value<0) {
      fprintf(stderr,
	"WARNING: negative value of %s found, using default.\n",variable);
      return(def);
    }
    if(s[strlen(s)-1]=='K') {
      value*=1024;
    } else if(s[strlen(s)-1]=='M') {
      value*=(1024*1024);
    }
    if(value >= minimal) { 
      return(value);
    } else {
      fprintf(stderr,
	"WARNING: %s smaller than minimum of %d, using default.\n",
	variable, minimal);
      return(def);
    }
  }
}

static int validate(char* domain,char *sn,unsigned char *key) {
  unsigned int next = 1;
  unsigned int i,j,v;
  v=sn[0]*256+sn[1]; 
  next = v;
  for(i=0; i<15; i++) {
    v=sn[i+0]*256+sn[i+1]; 
    next = next * 1103515245 + 12345 + v;
  }
  for(i=0; i<15; i++) {
    v=domain[i+0]*256+domain[i+1]; 
    next = next * 1103515245 + 12345 + v;
  }
  for(j=0; j<16; j++) {
    next = next * 1103515245 + 12345 + ((unsigned int) sn[j]);
  }
  for(j=16; j<32; j++) {
    next = next * 1103515245 + 12345 + ((unsigned int) sn[j-16]);
    if(((unsigned char) (key[j-16]^0xaa)) != (((next >> 16) & 255) % 26)+'A') {
      return((int)0);
    }
  }
  return((int)-1);
}

main(int argc,char** argv)
{
  SchemeAuxmemMax=askenv("SCHEME_AUXMEMSIZE",1*1024*1024,16*1024*1024);
  InitAtom(argc, argv);				
  InitStack(askenv("SCHEME_PSTACKSIZE",4096,16*1024),
	    askenv("SCHEME_CSTACKSIZE",4096,16*1024));	
  InitHeap((askenv("SCHEME_HEAPSIZE",2*1024*1024,4*1024*1024)/sizeof(SchemeWord))/2);
  InitAtomHash(askenv("SCHEME_HASHSIZE",4096,4096));	
  InitUtil();			
  InitStandardAtoms();
  InitMachine();
  SignalInit();
 
  SchemeBootPrimitives();
  SchemeRunMachine();

  SchemeRead();
  SchemePrint();
}

/*
void* realloc(void* x, size_t y) {
  fprintf(stderr,"ARG\n");
  exit(2);
}
*/
