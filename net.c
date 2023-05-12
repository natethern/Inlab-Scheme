/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / ALle Rechte vorbehalten
 * $Id: net.c,v 1.4 2006/04/29 09:13:22 tommy Exp $
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
#include <errno.h>

#include <netdb.h>                                                                                                    
#include <stdlib.h>


static char* rcsid="$Id: net.c,v 1.4 2006/04/29 09:13:22 tommy Exp $";

/*
 * Typedef des neuen Typen mit External am Anfang
 */

typedef struct {
  External external;
  int tcpsocket;
} TCPSOCKET;

/*
 * Definition: wie wird der Typ ausgegeben ? Ausgabe erfolgt ueber writestring.
 */

static if_tcpsocket_print(Port* p, TCPSOCKET* e) {
  char buf[128];
  char adr[128];

  if(e->tcpsocket != -1) {
    sprintf(buf,"#<tcp socket %d>", e->tcpsocket);
  } else {
    sprintf(buf,"#<tcp socket unknown>");
  }
  writestring(p,buf);
}

/*
 * Wie wird der Typ von der GC behandelt wenn keine Referenz mehr da ist ?
 */

static if_tcpsocket_destroy(TCPSOCKET* e) {
  if(e->tcpsocket != -1) {
    close(e->tcpsocket);
    e->tcpsocket=-1;
  }
}

/*
 * Funktionsliste identifiziert zugleich diesen Typ.
 */

static ExternalFunctions tcpsocket_functions={
  if_tcpsocket_print,
  if_tcpsocket_destroy,
  NULL,			/* putchar */
  NULL,			/* putstring */
  NULL,			/* getchar */
  NULL			/* ungetchar */
};



PrimitiveProcedure(tcp-connect,2)
{
  TCPSOCKET* e;
  struct hostent *hent;
  struct sockaddr_in serv_addr;
  int sockfd;
  struct servent* sp;

  if(!IS_STRING(ARG(0)) || !IS_STRING(ARG(1))) {
    werr("argument-error, strings expected");
    return;
  } else {

    e=(TCPSOCKET*) AllocExternalType((sizeof(TCPSOCKET) - sizeof(External))/sizeof(SchemeWord));
    e->external.functions=&tcpsocket_functions; 
    e->tcpsocket=-1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      werr("can't open stream socket");
      return;
    }  

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    hent = gethostbyname(C_STRING(ARG(1)));
    if(hent == NULL) {
      if((serv_addr.sin_addr.s_addr = inet_addr(C_STRING(ARG(1))))== -1) {
        werr("unknown or invalid address");
        return;
      }
    } else {
      memcpy(&serv_addr.sin_addr.s_addr, hent->h_addr, hent->h_length);
    }

    sp=getservbyname(C_STRING(ARG(0)),"tcp");
    if(sp == NULL) {
      serv_addr.sin_port = htons(atoi(C_STRING(ARG(0))));
    } else {
      serv_addr.sin_port = htons(sp->s_port);
    }

    printf("Port=%d\n", serv_addr.sin_port);
    fprintf(stderr,"%s\n",inet_ntoa(serv_addr.sin_addr.s_addr));

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {  
      perror("connect");
      werr("connection refused");
      return;
    }

    e->tcpsocket=sockfd;
    val=(Pair*) e;
  }
}




