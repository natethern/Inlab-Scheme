/* (c) Copyright 1991-2005,2006 by Inlab Software, GmbH Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: ports.c,v 4.105 2006/04/29 09:13:22 tommy Exp $
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <scheme.h>
#include <extern.h>
#include <global.h>
#include <fdecl.h>
#include <heap.h>

#define FGETSLEN 512	/* Bytebuffer bei read-until-char */

static char* rcsid = "$Id: ports.c,v 4.105 2006/04/29 09:13:22 tommy Exp $";

Port* createstdin()
{
  Port* p;
  p = (Port*) AllocInputPort();
  p->fd = stdin;
  p->character = -1;
  p->line = 0;
  p->col = 0;
  return(p);
}

Port* createstdout() 
{
  Port* p;
  p = (Port*) AllocOutputPort();
  p->fd = stdout;
  p->character = -1;
  p->line = 0;
  p->col = 0;
  return(p);
}

Port* createstderr()
{
  Port* p;
  p = (Port*) AllocOutputPort();
  p->fd = stderr;
  p->character = -1;
  p->line = 0;
  p->col = 0;
  return(p);
}

Port* createinputstringport()
{
  Port* p;
  p = (Port*) AllocInputStringPort();
  p->character = -1;
  p->line = 0;
  p->col = 0;
  ((InputStringPort*) p)->position = 0;
  ((InputStringPort*) p)->string = nil_constant;
  return(p); 
}

Port* createstaticport(char* start,char* end)
{
  Port* p;
  p=(Port*) AllocStaticPort();
  p->character = -1;
  p->line = 0;
  p->col = 0;
  ((StaticPort*) p)->current = start;
  ((StaticPort*) p)->invalid = end;
  return(p);
}

extern char* TheInternalFiles[][3];

Port* CreateStaticPortByName(char* name)
{
  int i=0;
  for(;;) {
    if(TheInternalFiles[i][0] == NULL) {
      return((Port*)false_constant);
    } else {
      if(!strcmp(TheInternalFiles[i][0],name)) {
	return(createstaticport(TheInternalFiles[i][1],
				TheInternalFiles[i][2]));
      } 
    }
    i++;
  }
}

Port* createoutputstringport()
{
  Port* p;
  p = (Port*) AllocOutputStringPort();
  p->character = -1;
  p->line = 0;
  p->col = 0;
  ((OutputStringPort*) p)->dummy = 0;
  ((OutputStringPort*) p)->charlist = nil_constant;
  return(p); 
}

closeport(Port* p)
{
  if (IS_OUTPUTPORT((Pair*) p) || IS_INPUTPORT((Pair*) p)) {
    switch(p->tag) {
      case INPUTPORT_TYPE:
      case OUTPUTPORT_TYPE:
	if(p->fd != NULL)
	  fclose(p->fd);
	p->fd = NULL;
	break;
      case POPENR_TYPE:
      case POPENW_TYPE:
	if(p->fd != NULL)
	  pclose(p->fd);
	p->fd = NULL;
	break;
      case INPUTSTRINGPORT_TYPE:
	((InputStringPort*) p)->string = nil_constant;
	break;
      case STATICPORT_TYPE:
	((StaticPort*) p)->character = -1;
	((StaticPort*) p)->current = NULL;
	((StaticPort*) p)->invalid = NULL;
	break;
      case OUTPUTSTRINGPORT_TYPE:
	((OutputStringPort*) p)->charlist = nil_constant;
	break;
      case EXTERNAL_TYPE:
        if(((External*) p)->functions != NULL) {
          if(((External*) p)->functions->getchar != NULL) {
            (*((External*) p)->functions->destroy)((External*) p);
          }
       }
       break;
    }
  } else {
    werr("cannot close port / no port");
  }
}

flushport(Port* p)
{
  if (p->tag != INPUTSTRINGPORT_TYPE && p->tag != OUTPUTSTRINGPORT_TYPE) {
    if (p->fd != NULL) {
      fflush(p->fd);
    }
  }
}

clearerrport(Port* p)
{
  if (p->tag != INPUTSTRINGPORT_TYPE && p->tag != OUTPUTSTRINGPORT_TYPE) {
    if (p->fd != NULL) {
      clearerr(p->fd);
    }
  }
}

Port* openinputfile(char* name)
{
  FILE* f;
  Port* p;
  f = fopen(name,"r");
  if (f == NULL) {
    werr("cannot open file");
    return((Port*) nil_constant);
  } else {
    p = createstdin();
    p->fd = f;
  }
  return(p);
}

/*
 *   SchemeINITFILE == init
 *   SCHEMEPATH  evtl. gesetzt.
 *   bin ist compiliert ....
 */

static char** string_split(char* s)
{
  char *n, *p;
  int count,i;
  char **rc;
  if((n = (char*) malloc(strlen(s)+1)) == NULL) {
    SchemePanic("malloc failed");
  }
  strcpy(n,s);
  p=n;
  count=1;
  while(*p != '\000') {
    if(*p == ':') {
      count++;
      *p='\000';
    }
    p++;
  }
  if((rc=(char**) malloc((count+1)*sizeof(char*))) == NULL) {
    SchemePanic("malloc failed");
  }
  rc[count]=NULL;
  p=n;
  for(i=0; i<count;i++) {
    rc[i]=p;
    while(*p != '\000') {
      p++;
    }
    p++;
  }
  return(rc);
}

Port* openinitfile()
{
  FILE* f=NULL;
  Port* p;
  char* path;
  char** the_paths;
  char** current_path;
  char buf[512];

  strcpy(buf,SchemeINITFILE);
  strcat(buf,".bin");
  if((p=CreateStaticPortByName(buf)) != (Port*) false_constant) {
    goto ifs_found;
  }
  strcpy(buf,SchemeINITFILE);
  strcat(buf,".scm");
  if((p=CreateStaticPortByName(buf)) != (Port*) false_constant) {
    goto ifs_found;
  }

  path=".";

  the_paths=string_split(path);
  current_path=the_paths;
  while(*current_path != NULL) {
    strcpy(buf,*current_path);
    strcat(buf,"/");
    strcat(buf,SchemeINITFILE);
    strcat(buf,".bin");
    if((f=fopen(buf,"r")) != NULL) {
      goto found;
    }
    strcpy(buf,*current_path);
    strcat(buf,"/");
    strcat(buf,SchemeINITFILE);
    strcat(buf,".scm");
    if((f=fopen(buf,"r")) != NULL) {
      goto found;
    }
    current_path++;
  }
  SchemePanic("cannot open init-file");
found:
  free(*the_paths);
  free(the_paths);
  p = createstdin();
  p->fd = f;
  return(p);
ifs_found:
  return(p);
}

Port* openoutputfile(char* name)
{
  FILE* f;
  Port* p;
  f = fopen(name,"w");
  if (f == NULL) {
    werr("cannot create file");
    return((Port*) nil_constant);
  } else {
    p = createstdout();
    p->fd = f;
  }
  return(p);
}

Port* openoutputfileappend(char* name)
{
  FILE* f;
  Port* p;
  f = fopen(name,"a");
  if (f == NULL) {
    werr("cannot create file");
    return((Port*) nil_constant);
  } else {
    p = createstdout();
    p->fd = f;
  }
  return(p);
}
 
Port* openpopenwrite(spec)
char *spec;
{
  FILE* f;
  Port* p;
  f = (FILE*) popen(spec,"w");
  if (f == NULL) {
    werr("cannot start popen/write");
    return((Port*) nil_constant);
  } else {
    p = createstdout();
    p->tag = POPENW_TYPE;
    p->fd = f;
  }
  return(p);
}
 
Port* openpopenread(char* spec)
{
  FILE* f;
  Port* p;
  f = (FILE*) popen(spec,"r");
  if (f == NULL) {
    werr("cannot start popen/read");
    return((Port*) nil_constant);
  } else {
    p = createstdin();
    p->tag = POPENR_TYPE;
    p->fd = f;
  }
  return(p);
}
 
writechar(Port* port,int c)
{
  if (IS_OUTPUTPORT((Pair*) port)) {
    if(port->tag == OUTPUTSTRINGPORT_TYPE) {
      /* val wird ueberschrieben */
      SetPairIn(val);
      val->car = S_CHAR(c);
      val->cdr = ((OutputStringPort*) port)->charlist;
      ((OutputStringPort*) port)->charlist=val;
    } else {
      if (port->fd != NULL)
        fputc(c, port->fd);
      else
	werr("cannot write to port");
    }
    switch(c) {
    case '\n':
      port->line++;
      port->col = 0;
      break;
    case '\r':
      port->col = 0;
      break;
    default:
      port->col++;
      break;
    }
  } else {
    werr("no output-port");
  }
}

writestring(Port* port,char* s)
{
  while(*s != '\000') {
    writechar(port,*s);
    s++;
  }
}

readchar(Port* port)
{
  int rc=EOF;
  if (IS_INPUTPORT((Pair*) port)) {
    if(port->character == -1) {
      if (port->tag == INPUTSTRINGPORT_TYPE) {
	if (((InputStringPort*) port)->string == nil_constant) {
	  rc = EOF;
	} else {
	  if(((InputStringPort*) port)->position >=
	     C_STRINGLEN(((InputStringPort*) port)->string)) {
	    rc = EOF;
	  } else {
	    rc = C_STRING(((InputStringPort*) port)->string)
		  [(((InputStringPort*) port)->position)];
	    (((InputStringPort*) port)->position)++;
	  }
	}
      } else if (port->tag == EXTERNAL_TYPE) {
        if(((External*) port)->functions != NULL) {
          if(((External*) port)->functions->getchar != NULL) {
            return((*((External*) port)->functions->getchar)((External*) port));
          } else {
            return(EOF);
          } 
        } else {
          return(EOF);
        }
      } else {
	if (port->tag == STATICPORT_TYPE) {
	  if (((StaticPort*) port)->current == NULL) {
	    werr("cannot read from static-port");
	  } else {
	    if(((StaticPort*) port)->current ==
	       ((StaticPort*) port)->invalid) {
	      rc = EOF;
	    } else {
	      rc = (int) (*(((StaticPort*) port)->current));
	      (((StaticPort*) port)->current)++;
	    }
	  }
	} else {
	  if (port->fd != NULL) {
            rc = fgetc(port->fd);
	  } else {
	    werr("cannot read from port");
	  }
	}
      }
    } else {
      rc = port->character;
      port->character = -1;
      return(rc);
    }
    switch(rc) {
      case '\n':
        port->line++;
        port->col = 0;
        break;
      case '\r':
        port->col = 0;
        break;
      default:
        port->col++;
        break;
    }
    return(rc);
  } else {
    werr("no input-port");
    return(EOF);
  }
}

ungetchar(Port* port,int c)
{
  if(IS_INPUTPORT((Pair*) port)) {
    if (port->character != -1) {
      werr("double ungetc to port not allowed");
    } else {
      port->character = c;
      switch(c) {
        case '\n':
          port->line--;
          break;
        case '\r':
          break;
        default:
          port->col--;
          break;
      }
    }
  } else {
    werr("cannot ungetc to output-port");
  }
}


/*
 *   readchar(port) liefert rcs wie fgetc() => EOF bei End of File ...
 */

char *readuntilchar(Port* port,int c,int* transfer)
{
  char* p;
  static char buf[FGETSLEN];
  int readc;

  p = buf;
  *transfer = 0;
  for(;;) {
    if (*transfer == FGETSLEN-1) {
      return(buf);
    }
    readc = readchar(port);
    if(readc == EOF) {
      return(buf);
    } else {
      *p++ = readc;
      *transfer = (*transfer)+1;
      if (readc == c) 
        return(buf);
    }
  }   
}



