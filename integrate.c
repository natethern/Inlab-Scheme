/* Copyright (c) 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: integrate.c,v 4.102 2006/04/29 09:13:22 tommy Exp $
 */

#include <stdio.h>
#include <stdlib.h>

static char* rcsid = "$Id: integrate.c,v 4.102 2006/04/29 09:13:22 tommy Exp $";

main(argc,argv)
int argc;
char **argv;
{
  int i,j;
  int c;
  FILE *fp;
  printf("%s","#include <stdio.h>\n");
  for(i=1; i<argc; i++) {
    /*
    fprintf(stderr,"Processing %s ...\n",argv[i]);
    */
    if((fp=fopen(argv[i],"r")) == NULL) {
      fprintf(stderr,"cannot open %s, exiting ...\n",argv[i]);
      exit(1);
    } else {
      printf("static const char txt%d[]={\n",i);
      for(;;) {
        for(j=0;j<10;j++) {
	  c=fgetc(fp);
	  if(c == EOF) {
	    break;
	  }
          printf("%d, ",c);	
        }
        printf("\n");
	if(c == EOF) {
	  break;
	}
      }
      printf("0\n");
      fclose(fp);
      printf("%s\n","};\n");
    }
  }
  printf("%s\n","const char* TheInternalFiles[][3]={");
  for(i=1; i<argc; i++) {
    printf("\"%s\",txt%d,txt%d+sizeof(txt%d)-1,\n",argv[i],i,i,i);
  }
  printf("NULL, NULL, NULL\n");
  printf("};\n");
  return(0);
}
