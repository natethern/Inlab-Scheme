/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: primitives.c,v 4.110 2006/04/29 09:13:22 tommy Exp $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <scheme.h>
#include <string.h>
#include <limits.h>
#include <extern.h>
#include <fdecl.h>
#include <global.h>
#include <heap.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/times.h>

static char* rcsid="$Id: primitives.c,v 4.110 2006/04/29 09:13:22 tommy Exp $";

static char * SchemeDateCreated=__DATE__;
static char * SchemeTimeCreated=__TIME__;

PrimitiveProcedure(car,1)
{
  if (ARG(0)->tag != PAIR_TYPE) {
    werr("argument-error");
  } else {
    val=ARG(0)->car;
  }
}

PrimitiveProcedure(special-car,1)
{
  if (!IS_SPECIALPAIR(ARG(0)) && ARG(0)->tag != PAIR_TYPE) {
    werr("argument-error");
  } else {
    val=ARG(0)->car;
  }
}

PrimitiveProcedure(environment-variable,1)
{
  if (ARG(0)->tag != ENVIRONMENT_TYPE) {
    werr("argument-error");
  } else {
    if (ARG(0)->tag != ENVIRONMENT_TYPE) {
      werr("environment expected");
    } else {
      val=((Environment*) ARG(0))->variable;
    }
  }
}

PrimitiveProcedure(environment-value,1)
{
  if (ARG(0)->tag != ENVIRONMENT_TYPE) {
    werr("argument-error");
  } else {
    if (ARG(0)->tag != ENVIRONMENT_TYPE) {
      werr("environment expected");
    } else {
      val=((Environment*) ARG(0))->value;
    }
  }
}

PrimitiveProcedure(environment-next,1)
{
  if (ARG(0)->tag != ENVIRONMENT_TYPE) {
    werr("argument-error");
  } else {
    if (ARG(0)->tag != ENVIRONMENT_TYPE) {
      werr("environment expected");
    } else {
      val=((Environment*) ARG(0))->next;
    }
  }
}

PrimitiveProcedure(cdr,1)
{
  if (ARG(0)->tag != PAIR_TYPE) {
    werr("argument-error");
  } else {
    val=ARG(0)->cdr;
  }
}

PrimitiveProcedure(special-cdr,1)
{
  if (!IS_SPECIALPAIR(ARG(0)) && ARG(0)->tag != PAIR_TYPE) {
    werr("argument-error");
  } else {
    val=ARG(0)->cdr;
  }
}

PrimitiveProcedure(cons,2)
{
  SetPairIn(val);
  val->car=ARG(1);
  val->cdr=ARG(0);
}

PrimitiveProcedure(fixarg-exit,1)
{
  if (ARG(0)->tag == INTEGER_TYPE) {
    exit((int) C_LONG(ARG(0)));
  } else {
    werr("argument-error");
  }
}

PrimitiveProcedure(internal-print,1)
{
  SchemePushP(ARG(0));
  SchemePrint();
  val=ok_atom;
}

PrimitiveProcedure(internal-write,1)
{
  SchemePushP(ARG(0));
  SchemeWrite();		/* wie print nur ohne newline am Ende ! */
  val=ok_atom;
}

PrimitiveProcedure(internal-display,1)
{
  SchemePushP(ARG(0));
  SchemeDisplay();
  val=ok_atom;
}

PrimitiveProcedure(null?,1)
{
  if (ARG(0) == nil_constant)
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(pair?,1)
{
    if (IS_PAIR(ARG(0)))
      val=true_constant;
    else
      val=false_constant;
}

PrimitiveProcedure(environment?,1)
{
    if (ARG(0)->tag == ENVIRONMENT_TYPE)
      val=true_constant;
    else
      val=false_constant;
}

PrimitiveProcedure(eq?,2)
{
  if (ARG(0) == ARG(1)) 
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(eqv?,2)
{
  if (IS_INTEGER(ARG(0)) || IS_INTEGER(ARG(1))) {
    if (C_INTEGER(ARG(0)) == C_INTEGER(ARG(1))) {
      val=true_constant;
    } else {
      val=false_constant;
    }
  } else if (IS_FLOAT(ARG(0)) || IS_FLOAT(ARG(1))) {
    if (C_FLOAT(ARG(0)) == C_FLOAT(ARG(1))) {
      val=true_constant;
    } else {
      val=false_constant;
    }
  } else {
    if (ARG(0) == ARG(1)) {
      val=true_constant;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(set-car!,2)
{
  if (!IS_PAIR(ARG(1))) {
    werr ("first arg not a pair");
    return;
  }
  ARG(1)->car=ARG(0);
  val=ok_atom;
}

PrimitiveProcedure(set-cdr!,2)
{
  if (!IS_PAIR(ARG(1))) {
    werr ("first args not a pair");
    return;
  }
  ARG(1)->cdr=ARG(0);
  val=ok_atom;
}

PrimitiveProcedure(not,1)
{
    if (ARG(0) == false_constant)
      val=true_constant;
    else
      val=false_constant;
}

PrimitiveProcedure(symbol?,1)
{
    if (IS_SYMBOL(ARG(0)))
      val=true_constant;
    else
      val=false_constant;
}

PrimitiveProcedure(vector?,1)
{
    if (IS_VECTOR(ARG(0)))
      val=true_constant;
    else
      val=false_constant;
}

PrimitiveProcedure(string?,1)
{
    if (IS_STRING(ARG(0)))
      val=true_constant;
    else
      val=false_constant;
}

PrimitiveProcedure(fixarg-make-vector,2)
{
  int	vector_length;
  int	i;
  Pair* fill;

    if (IS_INTEGER(ARG(1))) {
      vector_length=C_INTEGER(ARG(1));
      fill=ARG(0);
    } else {
      werr("integer expected");
      return;
    }
  val=make_vector(vector_length,fill);
}

PrimitiveProcedure(vector-length,1)
{
  if (!IS_VECTOR(ARG(0))) {
    werr ("no vector");
    return;
  } else {
    SetIntegerIn(val);
    val->car=(Pair*) ((SchemeWord) (C_VECTORLEN(ARG(0))));
  }
}

PrimitiveProcedure(vector-ref,2)
{
  int 	index;
  int   p_index;
  Pair* v;

  if (!IS_VECTOR(ARG(1)) || !IS_INTEGER(ARG(0))) {
    werr ("argument-error");
    return;
  } else {
    index=C_INTEGER(ARG(0));
    v    =ARG(1);
    if (index < 0 || index >= C_VECTORLEN(ARG(1))) {
      werr("invalid reference");
      return;
    } else {
      val=(C_VECTOR(ARG(1)))[index];
    }
  }
}

PrimitiveProcedure(vector-set!,3)
{
  int 	index;
  int   p_index;
  Pair* v;

  if (!IS_VECTOR(ARG(2)) || !IS_INTEGER(ARG(1))) {
    werr ("argument-error");
    return;
  } else {
    index=C_INTEGER(ARG(1));
    v=ARG(2);
    if (index < 0 || index >= C_VECTORLEN(ARG(2))) {
      werr("invalid reference");
      return;
    } else {
      (C_VECTOR(ARG(2)))[index]=ARG(0);
      val=ok_atom;
    }
  }
}

PrimitiveProcedure(gc,0)
{
  garbage_collect();
  val=ok_atom;
}

PrimitiveProcedure(internal-read,0)
{
  SchemeRead();
  val=SchemePopP();
}

PrimitiveProcedure(char?,1)
{
  if (IS_CHAR(ARG(0)))
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(char=?,2)
{
  if (!IS_CHAR(ARG(0)) || !IS_CHAR(ARG(1))) {
    werr ("no character");
    return;
  } else {
    if (C_CHAR(ARG(0)) == C_CHAR(ARG(1))) 
      val=true_constant;
    else
      val=false_constant;
  }
}

PrimitiveProcedure(char<?,2)
{
  if (!IS_CHAR(ARG(1)) || !IS_CHAR(ARG(0))) {
    werr ("no char");
    return;
  } else {
    if (C_CHAR(ARG(1)) < C_CHAR(ARG(0))) 
      val=true_constant;
    else
      val=false_constant;
  }
}

PrimitiveProcedure(char>?,2)
{
  if (!IS_CHAR(ARG(1)) || !IS_CHAR(ARG(0))) {
    werr ("no char");
    return;
  } else {
    if (C_CHAR(ARG(1)) > C_CHAR(ARG(0))) 
      val=true_constant;
    else
      val=false_constant;
  }
}

PrimitiveProcedure(char<=?,2)
{
  if (!IS_CHAR(ARG(1)) || !IS_CHAR(ARG(0))) {
    werr ("no char");
    return;
  } else {
    if (C_CHAR(ARG(1)) <= C_CHAR(ARG(0))) 
      val=true_constant;
    else
      val=false_constant;
  }
}

PrimitiveProcedure(char>=?,2)
{
  if (!IS_CHAR(ARG(1)) || !IS_CHAR(ARG(0))) {
    werr ("no char");
    return;
  } else {
    if (C_CHAR(ARG(1)) >= C_CHAR(ARG(0))) 
      val=true_constant;
    else
      val=false_constant;
  }
}

PrimitiveProcedure(char->integer,1)
{
  if (!IS_CHAR(ARG(0))) {
    werr("no char");
    return;
  } else {
    SetIntegerIn(val);
    val->car=(ARG(0))->car;
  }
}

PrimitiveProcedure(integer->char,1)
{
  int wert; 
  if (!IS_INTEGER(ARG(0))) {
    werr("no exact-integer");
    return;
  } else {
    wert=C_INTEGER(ARG(0));
    if (wert < 0 || wert > 255) {
      werr("invalid value");
      return;
    } else {
      val=S_CHAR(wert);
    }
  }
}

PrimitiveProcedure(fixarg-make-string,2)
{
  int	string_length;
  char  fill;

  if (IS_INTEGER(ARG(1))) {
    string_length=C_INTEGER(ARG(1));
    if (!IS_CHAR(ARG(0))) {
      werr("char expected");
      return;
    } else {
      fill=C_CHAR(ARG(0));
    }
  } else {
    werr("integer expected");
    return;
  }
  val=make_string(string_length,fill);
}

PrimitiveProcedure(string-length,1)
{
  int l;
  if (!IS_STRING(ARG(0))) {
    werr("string expected");
    return;
  } else {
    SetIntegerIn(val);
    l=C_STRINGLEN(ARG(0));
    val->car=(Pair*) ((SchemeWord) l); 
  }
}

PrimitiveProcedure(string-ref,2)
{
  unsigned char *p;
  int  l,index;

  if (!IS_STRING(ARG(1))) {
    werr("string expected");
    return;
  } else {
    if (!IS_INTEGER(ARG(0))) {
      werr("exact-integer expected");
      return;
    } else {
      p=C_STRING(ARG(1));
      index=C_INTEGER(ARG(0));
      l=C_STRINGLEN(ARG(1));        
      if (index < 0 || index >= l) {
	werr("invalid index");
	return;
      } else {
	val=S_CHAR(p[index]);
      }
    }
  }
}

PrimitiveProcedure(string-set!,3)
{
  unsigned char *p;
  int  l,index;

  if (!IS_STRING(ARG(2))) {
    werr("string expected");
    return;
  } else {
    if (!IS_INTEGER(ARG(1))) {
      werr("exact-integer expected");
      return;
    } else {
      if (!IS_CHAR(ARG(0))) {
	werr("char expected");
	return;
      } else {
	p=C_STRING(ARG(2));
	index=C_INTEGER(ARG(1));
	l=C_STRINGLEN(ARG(2));        
	if (index < 0 || index >= l) {
	  werr("invalid index");
	  return;
	} else {
	  p[index]=C_CHAR(ARG(0));
	  val=ok_atom;         
	}
      }
    }
  }
}

PrimitiveProcedure(internal-string-append,2)
{
  int l1, l2;		/* Laenge der beiden Strings */
  int i;
  unsigned char *p;
  unsigned char *s1;
  unsigned char *s2;

  if (!IS_STRING(ARG(1)) || !IS_STRING(ARG(0))) {
    werr("argument-error, strings expected");
    return;
  } else {
    l1=C_STRINGLEN(ARG(1));
    l2=C_STRINGLEN(ARG(0));
    fr1=make_string(l1 + l2, ' ');
    p=C_STRING(fr1);
    s1=C_STRING(ARG(1));
    s2=C_STRING(ARG(0));

    for (i=0; i<l1; i++) {
      *p++=*s1++;
    }
    for (i=0; i<l2; i++) {
      *p++=*s2++;
    }
    val=fr1;
    fr1=nil_constant;
  }
}

PrimitiveProcedure(substring,3)
{
  unsigned char* os;
  int osl;
  unsigned char* ns;
  int i,nsl;
  int start, end;

  if (!IS_STRING(ARG(2)) || !IS_INTEGER(ARG(1)) || !IS_INTEGER(ARG(0))) {
    werr("argument error");
    return;
  } else {
    osl=C_STRINGLEN(ARG(2));
    start=C_INTEGER(ARG(1));
    end  =C_INTEGER(ARG(0));

    if (start == 0 && end == 0) {
      val=make_string(0,' ');
      return;
    }

    if (start < 0 || start > osl || end < 0 || end > osl || start > end) {
      werr("invalid index");
      return;
    } else {
      nsl=end - start;
      fr1=make_string(nsl,' ');
      os=C_STRING(ARG(2));
      ns=C_STRING(fr1);
      for (i=start; i<end; i++) {
	*ns++=os[i];
      }
      val=fr1;
      fr1=nil_constant;
    }
  }
}

/*
 *  0 ==		Beide Strings identisch
 *  1 ==		s1 > s2
 * -1 ==	 	s1 < s2
 *
 */

static SchemeStringCompare(s1,l1,s2,l2)
unsigned char* s1;
int l1;
unsigned char* s2;
int l2;
{
  int index=0;
  for (;;) {
    if (index == l1 || index == l2) {
      if (index == l1 && index == l2) {
	return(0);
      } else {
	if (index == l1) {
	  return(-1);
	} else {
	  return(1);
	}
      }
    }
    if (s1[index] > s2[index])
      return(1);
    if (s1[index] < s2[index])
      return(-1);
    index++;
  }
}

PrimitiveProcedure(string=?, 2)
{
  unsigned char *s1, *s2;
  int l1,l2;
  if (!IS_STRING(ARG(1)) || !IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    s1=C_STRING(ARG(1));
    s2=C_STRING(ARG(0));
    l1=C_STRINGLEN(ARG(1));
    l2=C_STRINGLEN(ARG(0));
    if (SchemeStringCompare(s1,l1,s2,l2) == 0) {
      val=true_constant;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(string<?,2)
{
  unsigned char *s1, *s2;
  int l1,l2;
  if (!IS_STRING(ARG(1)) || !IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    s1=C_STRING(ARG(1));
    s2=C_STRING(ARG(0));
    l1=C_STRINGLEN(ARG(1));
    l2=C_STRINGLEN(ARG(0));
    if (SchemeStringCompare(s1,l1,s2,l2) < 0) {
      val=true_constant;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(string>?,2)
{
  unsigned char *s1, *s2;
  int l1,l2;
  if (!IS_STRING(ARG(1)) || !IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    s1=C_STRING(ARG(1));
    s2=C_STRING(ARG(0));
    l1=C_STRINGLEN(ARG(1));
    l2=C_STRINGLEN(ARG(0));
    if (SchemeStringCompare(s1,l1,s2,l2) > 0) {
      val=true_constant;
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(string-upcase,1)
{
  int i,l;
  unsigned char *source, *dest;
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    l=C_STRINGLEN(ARG(0));
    val=make_string(l, ' ');
    source=C_STRING(ARG(0));
    dest  =C_STRING(val);
    for (i=0; i < l; i++) {
      if(*source >= 'a' && *source <= 'z') {
	*dest++=(*source++) - 'a' + 'A';
      } else {
	*dest++=*source++;
      }
    }
  }
}

PrimitiveProcedure(string-downcase,1)
{
  int i,l;
  unsigned char *source, *dest;
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    l=C_STRINGLEN(ARG(0));
    val=make_string(l, ' ');
    source=C_STRING(ARG(0));
    dest  =C_STRING(val);
    for (i=0; i < l; i++) {
      if(*source >= 'A' && *source <= 'Z') {
	*dest++=(*source++) - 'A' + 'a';
      } else {
	*dest++=*source++;
      }
    }
  }
}

PrimitiveProcedure(internal-system,1)
{
  unsigned char* s;
  int sl;
  int rcode;
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    s=C_STRING(ARG(0));
    sl=C_STRINGLEN(ARG(0));
    rcode=system((char*) s);
    val=AllocInteger();
    val->car=(Pair*) (rcode / 256);
  }
}

PrimitiveProcedure(list->vector,1)
{
  if ((!IS_PAIR(ARG(0)) && !IS_NIL(ARG(0)))) {
    werr("argument-error");
    return;
  } else {
    val=list_to_vector(ARG(0));
  }
}

PrimitiveProcedure(vector->list,1)
{
  if (!IS_VECTOR(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    val=vector_to_list(ARG(0));
  }
}

PrimitiveProcedure(list->string,1)
{
  if ((!IS_PAIR(ARG(0)) && !IS_NIL(ARG(0)))) {
    werr("argument-error");
    return;
  } else {
    val=list_to_string(ARG(0));
  }
}

PrimitiveProcedure(string->list,1)
{
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    val=string_to_list(ARG(0));
  }
}

PrimitiveProcedure(length,1)
{
  if (!(IS_NIL(ARG(0)) || IS_PAIR(ARG(0)))) {
    werr("argument-error");
    return;
  }
  val=AllocInteger();
  val->car=(Pair*) (list_length(ARG(0)));
}

PrimitiveProcedure(internal-error,1)
{
  static char errbuf[MAXERRLEN];
  unsigned char* s;
  int sl;
  if (!(IS_STRING(ARG(0)))) {
    werr("string expected");
    return;
  }
  sl=C_STRINGLEN(ARG(0));
  if (sl >= MAXERRLEN) {
    werr("string too long");
    return;
  }
  s=C_STRING(ARG(0));
  strcpy(errbuf,(char*)s);
  werr(errbuf);
  val=nil_constant;
}

PrimitiveProcedure(direct-binding,1)
{
  if (!IS_SYMBOL(ARG(0))) {
    werr("argument-error");
    return;
  }
  val=((Atom*)(ARG(0)))->direct;
}

PrimitiveProcedure(internal-symbol->string,1)
{
  if (!IS_SYMBOL(ARG(0))) {
    werr("argument-error");
    return;
  }
  val=((Atom*) (ARG(0)))->pname;
}

PrimitiveProcedure(string->symbol,1)
{
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  }
  val=(Pair*) GetAtom (C_STRING(ARG(0)), C_STRINGLEN(ARG(0)));
}

PrimitiveProcedure(procedure?,1)
{
  if (ARG(0)->tag == COMPOUND_TYPE || 
      ARG(0)->tag == PRIMITIVE_TYPE ||
      ARG(0)->tag == CONTINUATION_TYPE)  
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(boolean?,1)
{
  if (ARG(0) == true_constant || ARG(0) == false_constant)      
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(number?,1)
{
  if (ARG(0)->tag == INTEGER_TYPE || ARG(0)->tag == FLOAT_TYPE)      
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(integer?,1)
{
  double d;
  if (ARG(0)->tag == INTEGER_TYPE) {    
    val=true_constant;
  } else {
    if (ARG(0)->tag == FLOAT_TYPE) {
      d=C_FLOAT(ARG(0));
      if ((d == floor(d)) && (d == ceil(d))) {
	val=true_constant;
      } else {
	val=false_constant;
      }
    } else {
      val=false_constant;
    }
  }
}

PrimitiveProcedure(exact?,1)
{
  if (ARG(0)->tag == INTEGER_TYPE) {    
    val=true_constant;
  } else {
    val=false_constant;
  }
}

PrimitiveProcedure(eof-object?,1)
{
  if (ARG(0) == end_of_file_atom)      
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(get-eof-object,0)
{
  val=end_of_file_atom;
}

PrimitiveProcedure(input-port?,1)
{
  if (IS_INPUTPORT(ARG(0)))      
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(output-port?,1)
{
  if (IS_OUTPUTPORT(ARG(0)))       
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(current-input-port,0)
{
  val=(Pair*) the_current_input_port;
}

PrimitiveProcedure(current-output-port,0)
{
  val=(Pair*) the_current_output_port;
}

PrimitiveProcedure(current-error-port,0)
{
  val=(Pair*) the_current_error_port;
}

PrimitiveProcedure(procedure-body,1)
{
  if (ARG(0)->tag != COMPOUND_TYPE && ARG(0)->tag != SYNTAXCOMPOUND_TYPE) 
    werr("argument-error");
  else
    val=((Compound*) ARG(0))->body;
}

PrimitiveProcedure(procedure-name,1)
{
  if(ARG(0)->tag == COMPOUND_TYPE || ARG(0)->tag == SYNTAXCOMPOUND_TYPE) {
    val=((Compound*) ARG(0))->name;
  } else if (ARG(0)->tag == PRIMITIVE_TYPE) {
    val=((Primitive*) ARG(0))->symbol;
  } else {
    werr("argument-error");
  }
}

PrimitiveProcedure(procedure-parameters,1)
{
  if (ARG(0)->tag != COMPOUND_TYPE && ARG(0)->tag != SYNTAXCOMPOUND_TYPE) 
    werr("argument-error");
  else
    val=((Compound*) ARG(0))->parameters;
}

PrimitiveProcedure(set-current-input-port!,1)
{
  if (!(IS_INPUTPORT(ARG(0)))) 
    werr("argument-error");
  else {
    the_current_input_port=(Port*) ARG(0);
  }
}

PrimitiveProcedure(set-current-output-port!,1)
{
  if (!(IS_OUTPUTPORT(ARG(0)))) 
    werr("argument-error");
  else {
    the_current_output_port=(Port*) ARG(0);
  }
}

PrimitiveProcedure(set-current-error-port!,1)
{
  if (!(IS_OUTPUTPORT(ARG(0)))) 
    werr("argument-error");
  else {
    the_current_error_port=(Port*) ARG(0);
  }
}

PrimitiveProcedure(internal-write-char,1)
{
  if (ARG(0)->tag != CHAR_TYPE)
    werr("argument-error");
  else {
    writechar(the_current_output_port,C_CHAR(ARG(0)));
    val=ok_atom;
  }
}

PrimitiveProcedure(internal-read-char,0)
{
  int rc;
  if ((rc=readchar(the_current_input_port)) == EOF)
    val=end_of_file_atom;
  else
    val=S_CHAR(rc);
}

PrimitiveProcedure(internal-read-until-char,1)
{
  unsigned char *p,*q;
  int  i,l;
  if (ARG(0)->tag != CHAR_TYPE) {
    werr("argument-error");
  } else {
    p=(unsigned char*)
         readuntilchar(the_current_input_port,C_CHAR(ARG(0)), &l);
    if (l == 0) {
      val=end_of_file_atom;
    } else {
      val=make_string(l,' ');
      q=C_STRING(val);
      for (i=0; i<l; i++) {
	*q++=*p++;
      }
    } 
  }
}

PrimitiveProcedure(internal-peek-char,0)
{
  int rc;
  if ((rc=readchar(the_current_input_port)) == EOF)
    val=end_of_file_atom;
  else {
    val=S_CHAR(rc);
    ungetchar(the_current_input_port, rc);
  }
}

PrimitiveProcedure(internal-open-input-file,1)
{
  unsigned char* s;
  int sl;
  int rcode;
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    s=C_STRING(ARG(0));
    sl=C_STRINGLEN(ARG(0));
    val=(Pair*) openinputfile(s);
    if (val == nil_constant) {
      werri("cannot open file",ARG(0));
    }
  }
}

PrimitiveProcedure(popen/r,1)
{
  unsigned char* s;
  int sl;
  int rcode;
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    s=C_STRING(ARG(0));
    sl=C_STRINGLEN(ARG(0));
    val=(Pair*) openpopenread(s);
    if (val == nil_constant) {
      werri("cannot start popen",ARG(0));
    }
  }
}

PrimitiveProcedure(internal-open-output-file-append,1)
{
  unsigned char* s;
  int sl;
  int rcode;
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    s=C_STRING(ARG(0));
    sl=C_STRINGLEN(ARG(0));
    val=(Pair*) openoutputfileappend(s);
    if (val == nil_constant) {
      werri("cannot create or append to file",ARG(0));
    }
  }
}

PrimitiveProcedure(open-internal-file,1)
{
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
  } else {
    val=(Pair*) CreateStaticPortByName(C_STRING(ARG(0)));
  }
}

PrimitiveProcedure(internal-open-output-file,1)
{
  unsigned char* s;
  int sl;
  int rcode;
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    s=C_STRING(ARG(0));
    sl=C_STRINGLEN(ARG(0));
    val=(Pair*) openoutputfile(s);
    if (val == nil_constant) {
      werri("cannot create file",ARG(0));
    }
  }
}

PrimitiveProcedure(popen/w,1)
{
  unsigned char* s;
  int sl;
  int rcode;
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    s=C_STRING(ARG(0));
    sl=C_STRINGLEN(ARG(0));
    val=(Pair*) openpopenwrite(s);
    if (val == nil_constant) {
      werri("cannot create file",ARG(0));
    }
  }
}

PrimitiveProcedure(close-input-port,1)
{
  if (!(IS_INPUTPORT(ARG(0)))) {
    werr("argument-error");
  } else {
    closeport(ARG(0));
    val=ok_atom;
  }
}

PrimitiveProcedure(close-output-port,1)
{
  if (!(IS_OUTPUTPORT(ARG(0)))) {
    werr("argument-error");
  } else {
    closeport(ARG(0));
    val=ok_atom;
  }
}

PrimitiveProcedure(flush-port,1)
{
  if (!(IS_OUTPUTPORT(ARG(0)))) {
    werr("argument-error");
  } else {
    flushport(ARG(0));
    val=ok_atom;
  }
}

/*
 * nur fuer Input-Port definiert.
 * (und auch nur dafuer gebraucht ...)
 *
 */

PrimitiveProcedure(clearerr-port,1)
{
  if (!(IS_INPUTPORT(ARG(0)))) {
    werr("argument-error");
  } else {
    clearerrport(ARG(0));
    val=ok_atom;
  }
}

PrimitiveProcedure(compound-procedure?,1)
{
  if (ARG(0)->tag == COMPOUND_TYPE)
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(syntax-procedure?,1)
{
  if (ARG(0)->tag == SYNTAXCOMPOUND_TYPE)
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(file-exists?,1)
{
  unsigned char* s;
  int sl;
  int rcode;
  FILE* f;
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    s=C_STRING(ARG(0));
    sl=C_STRINGLEN(ARG(0));
    f=fopen((char*)s,"r");
    if (f == NULL) {
      val=false_constant;
    } else {
      val=true_constant;
      fclose(f);
    }
  }
}

PrimitiveProcedure(primitive-procedure?,1)
{
  if (ARG(0)->tag == PRIMITIVE_TYPE)
    val=true_constant;
  else
    val=false_constant;
}

PrimitiveProcedure(internal-current-column,0)
{
  val=AllocInteger();
  val->car=(Pair*) (the_current_output_port->col); 
}

PrimitiveProcedure(internal-getenv,1)
{
  unsigned char* s;
  int sl;
  int rcode;
  char* x;
  if (!IS_STRING(ARG(0))) {
    werr("argument-error");
    return;
  } else {
    s=C_STRING(ARG(0));
    sl=C_STRINGLEN(ARG(0));
    if((x=getenv((char*) s)) == NULL) {
      val=false_constant;
    } else {
      sl=strlen(x);
      val=make_string(sl,' ');
      memcpy(C_STRING(val),x,sl);
    }
  }
}

PrimitiveProcedure(argument-vector,0)
{
  int i,l;
  Pair* tp;
  val=make_vector(global_argc,nil_constant);
  for(i=0; i<global_argc; i++) {
    l=strlen(global_argv[i]);
    tp=make_string(l,' '); /* hier kann val sich veraendern ! */
    C_VECTOR(val)[i]=tp; 
    memcpy(C_STRING(C_VECTOR(val)[i]),global_argv[i],l);
  }
}

PrimitiveProcedure(console-error-port,0)
{
  val=(Pair*) the_console_error_port;
}

PrimitiveProcedure(console-input-port,0)
{
  val=(Pair*) the_console_input_port;
}

PrimitiveProcedure(console-output-port,0)
{
  val=(Pair*) the_console_output_port;
}

matherr(x)
struct exception *x;
{
  werr("libm-error");
  return(1);
}

static double to_double(x)
Pair* x;
{
  double d;
  long l;
  int i;
  l=C_LONG(x);
  i=l;
  d=i;
  return(d);
}

static double sure_double(x)
Pair* x;
{
  double d;
  if(x->tag == FLOAT_TYPE)
    return(C_FLOAT(x));
  else if (x->tag == INTEGER_TYPE)
    return(d=C_LONG(x));
  else {
    werr("numeric type expected");
    return(0.0);
  }
}

static Pair* make_double(d)
double d;
{
  Pair* x;
  x=AllocFloat();
  C_FLOAT(x)=d;
  return(x);
}

static Pair* make_integer(l)
long l;
{
  Pair* x;
  SetIntegerIn(x);
  x->car=(Pair*) ((SchemeWord) l);
  return x;
}

PrimitiveProcedure(exact->inexact,1)
{
  if (ARG(0)->tag != INTEGER_TYPE) {
    werr("argument not exact or numeric");
  } else {
    val=make_double(sure_double(ARG(0)));
  }
}

PrimitiveProcedure(inexact->exact,1)
{
  double d;
  if (ARG(0)->tag != FLOAT_TYPE) {
    werr("argument not inexact or numeric");
  } else {
    d=C_FLOAT(ARG(0));
    if (d > ((double) LONG_MAX) || d < ((double) LONG_MIN)) {
      werr("inexact argument does not fit in exact");
    } else {
      val=make_integer((int) d);	
    }
  }
}

PrimitiveProcedure(cos,1)
{
  switch((int) ARG(0)->tag) {
  case FLOAT_TYPE:
    val=make_double(cos(C_FLOAT(ARG(0))));
    break;
  case INTEGER_TYPE:
    val=make_double(cos(sure_double(ARG(0))));
    break;
  default:
    werr("numeric argument expected");
    break;
  }
}

PrimitiveProcedure(exp,1)
{
  switch((int) ARG(0)->tag) {
  case FLOAT_TYPE:
    val=make_double(exp(C_FLOAT(ARG(0))));
    break;
  case INTEGER_TYPE:
    val=make_double(exp(sure_double(ARG(0))));
    break;
  default:
    werr("numeric argument expected");
    break;
  }
}

PrimitiveProcedure(expt,2)
{
  if ((ARG(1)->tag != INTEGER_TYPE && ARG(1)->tag != FLOAT_TYPE) ||
      (ARG(0)->tag != INTEGER_TYPE && ARG(0)->tag != FLOAT_TYPE)) {
    werr("argument-error");
  } else {
    val=make_double(pow(sure_double(ARG(1)),sure_double(ARG(0))));
  }
}

PrimitiveProcedure(sqrt,1)
{
  if ((ARG(0)->tag != INTEGER_TYPE && ARG(0)->tag != FLOAT_TYPE)) {
    werr("argument-error");
  } else {
    val=make_double(sqrt(sure_double(ARG(0))));
  }
}

PrimitiveProcedure(floor,1)
{
  switch((int) ARG(0)->tag) {
  case FLOAT_TYPE:
    val=make_double(floor(C_FLOAT(ARG(0))));
    break;
  case INTEGER_TYPE:
    val=make_double(floor(sure_double(ARG(0))));
    break;
  default:
    werr("numeric argument expected");
    break;
  }
}

PrimitiveProcedure(ceiling,1)
{
  switch((int) ARG(0)->tag) {
  case FLOAT_TYPE:
    val=make_double(ceil(C_FLOAT(ARG(0))));
    break;
  case INTEGER_TYPE:
    val=make_double(ceil(sure_double(ARG(0))));
    break;
  default:
    werr("numeric argument expected");
    break;
  }
}

/* MURX */
double rint(double);

PrimitiveProcedure(round,1)
{
  switch((int) ARG(0)->tag) {
  case FLOAT_TYPE:
    val=make_double(rint(C_FLOAT(ARG(0))));
    break;
  case INTEGER_TYPE:
    val=ARG(0); 
    break;
  default:
    werr("numeric argument expected");
    break;
  }
}

PrimitiveProcedure(internal-/,2)
{
  static char* nonum="numeric argument expected";
  long x,y,z;
  switch((int) ARG(1)->tag) {
  case INTEGER_TYPE:
    switch ((int) ARG(0)->tag) {
      case INTEGER_TYPE:
	x=C_LONG(ARG(1));
	y=C_LONG(ARG(0));
	if (y == 0) {
	  werr("division by zero");
	} else {
	  z=x / y;
	  if (x % y  == 0) {
	    val=make_integer(z); 
	  } else {
	    val=make_double(((double) x) / ((double) y));
	  }
	}
	break;
      case FLOAT_TYPE:
	if (C_FLOAT(ARG(0)) == 0.0) {
	  werr("division by zero");
	} else {
	  val=make_double(to_double(ARG(1)) / C_FLOAT(ARG(0)));
	}
	break;
      default:
	werr(nonum);
	break;
    }
    break;
  case FLOAT_TYPE:
    switch ((int) ARG(0)->tag) {
      case INTEGER_TYPE:
	if (C_INTEGER(ARG(0)) == 0) {
	  werr("division by zero");
	} else {
	  val=make_double(C_FLOAT(ARG(1)) / to_double(ARG(0)));
	}
	break;
      case FLOAT_TYPE:
	if (C_FLOAT(ARG(0)) == 0.0) {
	  werr("division by zero");
	} else {
	  val=make_double(C_FLOAT(ARG(1)) / C_FLOAT(ARG(0)));
	}
	break;
      default:
	werr(nonum);
	break;
    }
    break;
  default:
    werr(nonum);
    break;
  }
}

PrimitiveProcedure(internal-*,2)
{
  static char* nonum="numeric argument expected / internal-*";
  switch ((int) ARG(0)->tag) {
  case INTEGER_TYPE:
    switch ((int) ARG(1)->tag) {
    case INTEGER_TYPE:
      if (C_LONG(ARG(0)) < 1000 && C_LONG(ARG(0)) > -1000 &&
	  C_LONG(ARG(1)) < 1000 && C_LONG(ARG(0)) > -1000) {
	val=make_integer(C_LONG(ARG(0)) * C_LONG(ARG(1))); 
      } else {
	val=make_double(to_double(ARG(0)) * to_double(ARG(1)));
      }
      break;
    case FLOAT_TYPE:
      if (C_INTEGER(ARG(0)) == 0L)
        val=make_integer(0L);
      else
        val=make_double(to_double(ARG(0)) * C_FLOAT(ARG(1)));
      break;
    default:
      werr(nonum);
      break;
    }
    break;
  case FLOAT_TYPE:
    switch ((int) ARG(1)->tag) {
    case INTEGER_TYPE:
      if (C_LONG(ARG(1)) == 0L)
	val=make_integer(0L);
      else
        val=make_double(C_FLOAT(ARG(0)) * to_double(ARG(1)));
      break;
    case FLOAT_TYPE:
      val=make_double(C_FLOAT(ARG(0)) * C_FLOAT(ARG(1)));
      break;
    default:
      werr(nonum);
      break;
    }
    break;
  default:
    werr(nonum);
    break;
  }
}

PrimitiveProcedure(internal-+,2)
{
  static char* nonum="numeric argument expected / internal-+";
  switch ((int) ARG(0)->tag) {
  case INTEGER_TYPE:
    switch ((int) ARG(1)->tag) {
    case INTEGER_TYPE:
      if (C_LONG(ARG(0)) < (LONG_MAX / 2) && C_LONG(ARG(0)) > (LONG_MIN / 2)
	 && C_LONG(ARG(1)) < (LONG_MAX / 2) && C_LONG(ARG(1)) > (LONG_MIN / 2)) {
	val=make_integer(C_LONG(ARG(0)) + C_LONG(ARG(1))); 
      } else {
	val=make_double(to_double(ARG(0)) + to_double(ARG(1)));
      }
      break;
    case FLOAT_TYPE:
      val=make_double(to_double(ARG(0)) + C_FLOAT(ARG(1)));
      break;
    default:
      werr(nonum);
      break;
    }
    break;
  case FLOAT_TYPE:
    switch ((int) ARG(1)->tag) {
    case INTEGER_TYPE:
      val=make_double(C_FLOAT(ARG(0)) + to_double(ARG(1)));
      break;
    case FLOAT_TYPE:
      val=make_double(C_FLOAT(ARG(0)) + C_FLOAT(ARG(1)));
      break;
    default:
      werr(nonum);
      break;
    }
    break;
  default:
    werr(nonum);
    break;
  }
}

PrimitiveProcedure(internal--,2)
{
  static char* nonum="numeric argument expected / internal--";
  switch ((int) ARG(0)->tag) {
  case INTEGER_TYPE:
    switch((int) ARG(1)->tag) {
    case INTEGER_TYPE:
      if (C_LONG(ARG(1)) < (LONG_MAX / 2) && C_LONG(ARG(1)) > (LONG_MIN / 2)
	 && C_LONG(ARG(0)) < (LONG_MAX / 2) && C_LONG(ARG(0)) > (LONG_MIN / 2)) {
	val=make_integer(C_LONG(ARG(1)) - C_LONG(ARG(0))); 
      } else {
	val=make_double(to_double(ARG(1)) - to_double(ARG(0)));
      }
      break;
    case FLOAT_TYPE:
      val=make_double(C_FLOAT(ARG(1)) - to_double(ARG(0)));
      break;
    default:
      werr(nonum);
      break;
    }
    break;
  case FLOAT_TYPE:
    /*
     * Hier ist ARG(0) ein double !!!
     */
    switch ((int) ARG(1)->tag) {
    case INTEGER_TYPE:
      val=make_double(to_double(ARG(1)) - C_FLOAT(ARG(0)));
      break;
    case FLOAT_TYPE:
      val=make_double(C_FLOAT(ARG(1)) - C_FLOAT(ARG(0)));
      break;
    default:
      werr(nonum);
      break;
    }
    break;
  default:
    werr(nonum);
    break;
  }
}

PrimitiveProcedure(sin,1)
{
  switch((int) ARG(0)->tag) {
  case FLOAT_TYPE:
    val=make_double(sin(C_FLOAT(ARG(0))));
    break;
  case INTEGER_TYPE:
    val=make_double(sin(sure_double(ARG(0))));
    break;
  default:
    werr("numeric argument expected");
    break;
  }
}

PrimitiveProcedure(log,1)
{
  switch((int) ARG(0)->tag) {
  case FLOAT_TYPE:
    val=make_double(log(C_FLOAT(ARG(0))));
    break;
  case INTEGER_TYPE:
    val=make_double(log(sure_double(ARG(0))));
    break;
  default:
    werr("numeric argument expected");
    break;
  }
}

PrimitiveProcedure(tan,1)
{
  switch((int) ARG(0)->tag) {
  case FLOAT_TYPE:
    val=make_double(tan(C_FLOAT(ARG(0))));
    break;
  case INTEGER_TYPE:
    val=make_double(tan(sure_double(ARG(0))));
    break;
  default:
    werr("numeric argument expected");
    break;
  }
}

PrimitiveProcedure(asin,1)
{
  switch((int) ARG(0)->tag) {
  case FLOAT_TYPE:
    val=make_double(asin(C_FLOAT(ARG(0))));
    break;
  case INTEGER_TYPE:
    val=make_double(asin(sure_double(ARG(0))));
    break;
  default:
    werr("numeric argument expected");
    break;
  }
}

PrimitiveProcedure(acos,1)
{
  switch((int) ARG(0)->tag) {
  case FLOAT_TYPE:
    val=make_double(acos(C_FLOAT(ARG(0))));
    break;
  case INTEGER_TYPE:
    val=make_double(acos(sure_double(ARG(0))));
    break;
  default:
    werr("numeric argument expected");
    break;
  }
}

PrimitiveProcedure(internal-atan1,1)
{
  switch((int) ARG(0)->tag) {
  case FLOAT_TYPE:
    val=make_double(atan(C_FLOAT(ARG(0))));
    break;
  case INTEGER_TYPE:
    val=make_double(atan(sure_double(ARG(0))));
    break;
  default:
    werr("numeric argument expected");
    break;
  }
}

PrimitiveProcedure(internal-atan2,2)
{
  if ((ARG(1)->tag == INTEGER_TYPE || ARG(1)->tag == FLOAT_TYPE) &&
      (ARG(0)->tag == INTEGER_TYPE || ARG(0)->tag == FLOAT_TYPE)) {
    val=make_double(atan2(sure_double(ARG(1)), sure_double(ARG(0))));
  } else {
    werr("argument-error");
  }
}

PrimitiveProcedure(internal-number->string,2)
{
  char buf[64];
  char buf2[64];
  char * cp;
  if (ARG(0)->tag != INTEGER_TYPE || (ARG(1)->tag != FLOAT_TYPE && 
				      ARG(1)->tag != INTEGER_TYPE)) {
    werr("argument-error");
  } else {
    if (ARG(1)->tag == INTEGER_TYPE) {
      switch ((int) (C_LONG(ARG(0)))) {
	case 2:
	  sprintf(buf2,"%lx",C_LONG(ARG(1)));
	  strcpy(buf,"");
	  for (cp=buf2; *cp != '\000'; cp ++) {
	    switch (*cp) {
	      case '0':
		strcat(buf,"0000");
		break;
	      case '1':
		strcat(buf,"0001");
		break;
	      case '2':
		strcat(buf,"0010");
		break;
	      case '3':
		strcat(buf,"0011");
		break;
	      case '4':
		strcat(buf,"0100");
		break;
	      case '5':
		strcat(buf,"0101");
		break;
	      case '6':
		strcat(buf,"0110");
		break;
	      case '7':
		strcat(buf,"0111");
		break;
	      case '8':
		strcat(buf,"1000");
		break;
	      case '9':
		strcat(buf,"1001");
		break;
	      case 'a':
	      case 'A':
		strcat(buf,"1010");
		break;
	      case 'b':
	      case 'B':
		strcat(buf,"1011");
		break;
	      case 'c':
	      case 'C':
		strcat(buf,"1100");
		break;
	      case 'd':
	      case 'D':
		strcat(buf,"1101");
		break;
	      case 'e':
	      case 'E':
		strcat(buf,"1110");
		break;
	      case 'f':
	      case 'F':
		strcat(buf,"1111");
		break;
	      default:
		SchemePanic("invalid char / number->string INTERN");
		break;
	    }
	  }
	  break;
	case 8:
	  sprintf(buf,"%lo",C_LONG(ARG(1)));
	  break;
	case 10:
	  sprintf(buf,"%ld",C_LONG(ARG(1)));
	  break;
	case 16:
	  sprintf(buf,"%lx",C_LONG(ARG(1)));
	  break;
	default:
	  werr("invalid radix");
	  strcpy(buf,"");
	  break;
      }
      val=make_string(strlen(buf),' ');
      strcpy((char*) (C_STRING(val)),buf);
    } else {
      if (C_LONG(ARG(0)) != 10L) {
	werr("only radix 10 valid converting inexact reals");
      } else {
	sprintf(buf,"%g",C_FLOAT(ARG(1)));
	if (strchr(buf,'.') == NULL) {
	  strcat(buf,".0");
	}
	val=make_string(strlen(buf),' ');
	strcpy((char*) (C_STRING(val)),buf);
      }
    }
  }
}

static errf(ignored)
char *ignored;
{}

PrimitiveProcedure(internal-string->number,2)
{
  long l;
  double d;
  if (ARG(1)->tag != STRING_TYPE || ARG(0)->tag != INTEGER_TYPE) {
    werr("argument-error");
  } else {
    switch (StringToNumber(C_STRING(ARG(1)),&l, &d, (int) (C_LONG(ARG(0))), 
	      errf)) {
    case SchemeLONG:
      SetIntegerIn(val);
      val->car=(Pair*) l;
      break;
    case SchemeDOUBLE:
      val=AllocFloat();
      C_FLOAT(val)=d;
      break;
    case SchemeNONUMBER:
      val=false_constant;
      break;
    default:
      SchemePanic("invalid rcode StringToNumber / string->number");
      break;
    }
  }
}

PrimitiveProcedure(internal-=,2)
{
  if (ARG(1)->tag == INTEGER_TYPE && ARG(0)->tag == INTEGER_TYPE) {
    if (C_LONG(ARG(1)) == C_LONG(ARG(0))) 
      val=true_constant;
    else
      val=false_constant;
  } else {
    if(sure_double(ARG(1)) == sure_double(ARG(0)))
      val=true_constant;
    else
      val=false_constant;
  }
}

PrimitiveProcedure(internal->,2)
{
  if (ARG(1)->tag == INTEGER_TYPE && ARG(0)->tag == INTEGER_TYPE) {
    if (C_LONG(ARG(1)) > C_LONG(ARG(0))) 
      val=true_constant;
    else
      val=false_constant;
  } else {
    if(sure_double(ARG(1)) > sure_double(ARG(0)))
      val=true_constant;
    else
      val=false_constant;
  }
}

PrimitiveProcedure(internal-<,2)
{
  if (ARG(1)->tag == INTEGER_TYPE && ARG(0)->tag == INTEGER_TYPE) {
    if (C_LONG(ARG(1)) < C_LONG(ARG(0))) 
      val=true_constant;
    else
      val=false_constant;
  } else {
    if(sure_double(ARG(1)) < sure_double(ARG(0)))
      val=true_constant;
    else
      val=false_constant;
  }
}

PrimitiveProcedure(odd?,1)
{
  long x;
  if (ARG(0)->tag != INTEGER_TYPE) {
    werr("argument-error");
  } else {
    x=C_LONG(ARG(0));
    if (x % 2 == 0)
      val=false_constant;
    else
      val=true_constant;
  }
}

PrimitiveProcedure(even?,1)
{
  long x;
  if (ARG(0)->tag != INTEGER_TYPE) {
    werr("argument-error");
  } else {
    x=C_LONG(ARG(0));
    if (x % 2 == 0)
      val=true_constant;
    else
      val=false_constant;
  }
}

PrimitiveProcedure(remainder,2)
{
  long a,b,c;
  long sign=1;
  if (ARG(1)->tag != INTEGER_TYPE || ARG(0)->tag != INTEGER_TYPE) {
    werr("argument-error");
  } else {
    a=(long) ARG(1)->car;
    b=(long) ARG(0)->car;
    c=a - b * (a / b);
    if ((a < 0 && c > 0) || (a > 0 && c < 0))
      c *= -1;
    SetIntegerIn(val);
    val->car=(Pair*) c;
  }
}

PrimitiveProcedure(quotient,2)
{
  long a,b;
  if (ARG(1)->tag != INTEGER_TYPE || ARG(0)->tag != INTEGER_TYPE) {
    werr("argument-error");
  } else {
    a=(long) ARG(1)->car;
    b=(long) ARG(0)->car;
    SetIntegerIn(val);
    val->car=(Pair*) (a / b);
  }
}

PrimitiveProcedure(binary-not,1)
{
  if (ARG(0)->tag == INTEGER_TYPE) {
    SetIntegerIn(val);
    val->car=(Pair*) ((SchemeWord) (~ C_INTEGER(ARG(0))));
  } else if (ARG(0)->tag == CHAR_TYPE) {
    val=S_CHAR((~ C_CHAR(ARG(0))) & 255);
  } else {
    werr("unexpected argument type");
  }
}

PrimitiveProcedure(binary-and,2)
{
  if (ARG(0)->tag == INTEGER_TYPE && ARG(1)->tag == INTEGER_TYPE) {
    SetIntegerIn(val);
    val->car=(Pair*) (C_INTEGER(ARG(0)) & C_INTEGER(ARG(1)));
  } else if (ARG(0)->tag == CHAR_TYPE && ARG(1)->tag == CHAR_TYPE) {
    val=S_CHAR((C_CHAR(ARG(0)) & C_CHAR(ARG(1))) & 255);
  } else {
    werr("argument-error");
  }
}

PrimitiveProcedure(binary-or,2)
{
  if (ARG(0)->tag == INTEGER_TYPE && ARG(1)->tag == INTEGER_TYPE) {
    SetIntegerIn(val);
    val->car=(Pair*) (C_INTEGER(ARG(0)) | C_INTEGER(ARG(1)));
  } else if (ARG(0)->tag == CHAR_TYPE && ARG(1)->tag == CHAR_TYPE) {
    val=S_CHAR((C_CHAR(ARG(0)) | C_CHAR(ARG(1))) & 255);
  } else {
    werr("argument-error");
  }
}

PrimitiveProcedure(binary-xor,2)
{
  if (ARG(0)->tag == INTEGER_TYPE && ARG(1)->tag == INTEGER_TYPE) {
    SetIntegerIn(val);
    val->car=(Pair*) (C_INTEGER(ARG(0)) ^ C_INTEGER(ARG(1)));
  } else if (ARG(0)->tag == CHAR_TYPE && ARG(1)->tag == CHAR_TYPE) {
    val=S_CHAR((C_CHAR(ARG(0)) ^ C_CHAR(ARG(1))) & 255);
  } else {
    werr("argument-error");
  }
}

PrimitiveProcedure(1-,1)
{
  SchemeWord x;
  if(ARG(0)->tag != INTEGER_TYPE) {
    werri("argument error", argl);
  } else {
    x=((SchemeWord) (C_INTEGER(ARG(0))))-1;
    MakeInteger(val,x);
  }
}

PrimitiveProcedure(1+,1)
{
  SchemeWord x;
  if(ARG(0)->tag != INTEGER_TYPE) {
    werri("argument error", argl);
  } else {
    x=((SchemeWord) (C_INTEGER(ARG(0))))+1;
    MakeInteger(val,x);
  }
}

PrimitiveProcedure(symbol-exists?,1)
{
  if (ARG(0)->tag != STRING_TYPE) {
    werr("invalid parameter");
  } else {
    if (SearchAtom(C_STRING(ARG(0)), C_STRINGLEN(ARG(0))) == NULL)
      val=false_constant;
    else
      val=true_constant;
  }
}

PrimitiveProcedure(make-usertype,2)
{
  if (ARG(1)->tag != SYMBOL_TYPE) {
    werr("invalid parameter");
  } else {
    SetPairIn(val);
    val->tag=USER_TYPE;
    val->car=ARG(1);
    val->cdr=ARG(0);
  }
}

PrimitiveProcedure(get-usertype-type,1)
{
  if (ARG(0)->tag != USER_TYPE) {
    werr("invalid parameter");
  } else {
    val=ARG(0)->car;
  }
}

PrimitiveProcedure(get-usertype-data,1)
{
  if (ARG(0)->tag != USER_TYPE) {
    werr("invalid parameter");
  } else {
    val=ARG(0)->cdr;
  }
}

PrimitiveProcedure(set-usertype-type!,2)
{
  if (ARG(1)->tag != USER_TYPE || ARG(0)->tag != SYMBOL_TYPE) {
    werr("invalid parameter");
  } else {
    ARG(1)->car=ARG(0);
    val=ok_atom;
  }
}

PrimitiveProcedure(set-usertype-date!,2)
{
  if (ARG(1)->tag != USER_TYPE) {
    werr("invalid parameter");
  } else {
    ARG(1)->cdr=ARG(0);
    val=ok_atom;
  }
}

PrimitiveProcedure(usertype?,1)
{
  if (ARG(0)->tag == USER_TYPE) 
    val=true_constant;
  else 
    val=false_constant;
}

/*
 * kehrt nicht zurueck bei zirkulaeren Strukturen: 
 *
 */

PrimitiveProcedure(plain-list?,1)
{
  val=ARG(0);
  for (;;) {
    if (val == nil_constant) {
      val=true_constant;
      break;
    } else {
      if (val->tag == PAIR_TYPE) {
	if (val->car->tag != PAIR_TYPE && val->car->tag != VECTOR_TYPE) {
	  val=val->cdr;
	  continue;
	} else {
	  val=false_constant;
	  break;
	}
      } else {
	val=false_constant;
	break;
      }
    }
  }
}

PrimitiveProcedure(get-current-environment,0)
{
  val=env;
}

PrimitiveProcedure(get-global-environment,0)
{
  val=the_global_env;
}

PrimitiveProcedure(getpid,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) getpid());
}

PrimitiveProcedure(delete-file,1)
{
  if (ARG(0)->tag != STRING_TYPE) {
    werr("argument-error");
  } else {
    if(unlink(C_STRING(ARG(0)))) {
      werri ("error deleting-file",ARG(0));
    } else { 
      val=ok_atom;
    }
  }
}

PrimitiveProcedure(heap-size,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) HeapSize);
}

PrimitiveProcedure(internal-wordsize,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) sizeof(SchemeWord));
}


PrimitiveProcedure(auxmem-size,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) SchemeGetAuxmem());
}

PrimitiveProcedure(number-of-available-fds,0)
{
  int fds[2048];
  int i=0;
  int j;

  for(;;) {
    if((fds[i]=open("/dev/null", O_WRONLY)) == -1) {
      break;
    }
    i++;
  }
  for(j=0; j<i; j++)
    close(fds[j]);

  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) i);
}

PrimitiveProcedure(auxmem-maxsize,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) SchemeGetAuxmemMax());
}

PrimitiveProcedure(heap-used,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) HeapUsed);
}

PrimitiveProcedure(get-pstack-ptr,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) (the_pstackptr-the_pstack));
}

PrimitiveProcedure(get-pstack-size,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) (the_pstacksize));
}

PrimitiveProcedure(get-cstack-size,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) (the_cstacksize));
}

PrimitiveProcedure(get-cstack-ptr,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) (the_cstackptr-the_cstack));
}

PrimitiveProcedure(get-time,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) time(NULL));
}

PrimitiveProcedure(get-times,0)
{
  struct tms buffer;
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) times(&buffer));
}

PrimitiveProcedure(string-find-next-char,2)
{
  int i,l,c;
  unsigned char *p;
  if(ARG(1)->tag != STRING_TYPE || ARG(0)->tag != CHAR_TYPE) {
    werri("argument error",argl);
  } else {
    l=C_STRINGLEN(ARG(1));
    p=C_STRING(ARG(1));
    c=C_CHAR(ARG(0));
    for(i=0; i<l; i++) {
      if(c==p[i]) {
        val=AllocInteger();
        val->car=(Pair*) ((SchemeWord) i);
	return;
      }
    } 
    val=false_constant;
    return;
  }
}

PrimitiveProcedure(string-find-previous-char,2)
{
  int i,l,c;
  unsigned char *p;
  if(ARG(1)->tag != STRING_TYPE || ARG(0)->tag != CHAR_TYPE) {
    werr("argument error");
  } else {
    l=C_STRINGLEN(ARG(1));
    p=C_STRING(ARG(1));
    c=C_CHAR(ARG(0));
    for(i=l-1; i>=0; i--) {
      if(c==p[i]) {
        val=AllocInteger();
        val->car=(Pair*) ((SchemeWord) i);
 	return;
      }
    } 
    val=false_constant;
    return;
  }
}

PrimitiveProcedure(rand,0)
{
  val=AllocInteger();
  val->car=(Pair*) rand();
}

PrimitiveProcedure(srand,1)
{
  if (ARG(0)->tag != INTEGER_TYPE) {
    werr("argument-error");
  } else {
    srand(C_INTEGER(ARG(0)));
    val=ok_atom;
  }
}

PrimitiveProcedure(get-unassigned-constant,0)
{
  val=unassigned_constant;
}

PrimitiveProcedure(cd,1)
{
  if (ARG(0)->tag != STRING_TYPE) {
    werr("argument-error");
  } else {
    if (chdir(C_STRING(ARG(0))) != 0) {
      werri("cannot cd to directory",ARG(0));
    }
    val=ok_atom;
  }
}

PrimitiveProcedure(internal-set-expr-env-pair!,1)
{
  if (ARG(0)->tag != PAIR_TYPE 
		|| ARG(0)->cdr->tag != ENVIRONMENT_TYPE) {
    werr("argument-error");
  } else {
    expr_env=ARG(0);
    val=ok_atom;
  }
}

PrimitiveProcedure(get-total-symbols,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) GetTotalAtoms());
}

PrimitiveProcedure(get-free-symbols,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) GetFreeAtoms());
}

PrimitiveProcedure(get-hashtable-size,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) get_hashtable_size());
}

PrimitiveProcedure(get-hashtable-used,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) get_hashtable_used());
}

PrimitiveProcedure(get-hashtable-max,0)
{
  val=AllocInteger();
  val->car=(Pair*) ((SchemeWord) get_hashtable_max());
}

PrimitiveProcedure(enable-interrupt,0)
{
  SignalEnableInterrupts();
  val=ok_atom;
}

PrimitiveProcedure(disable-interrupt,0)
{
  SignalDisableInterrupts();
  val=ok_atom;
}

PrimitiveProcedure(error-handler-get-error-message,0)
{
  val=errormessage;
}

PrimitiveProcedure(error-handler-get-error-irritant,0)
{
  val=errorirritant;
}

PrimitiveProcedure(error-handler-get-error-callchain,0)
{
  val=errorcallchain;
}

PrimitiveProcedure(error-handler-get-error-environment,0)
{
  val=errorenvironment;
}

PrimitiveProcedure(error-handler-get-error-expression,0)
{
  val=errorexpression;
}

PrimitiveProcedure(error-handler-get-jumpout,0)
{
  val=jumpout;
}

PrimitiveProcedure(error-handler-get-jumpin,0)
{
  val=jumpin;
}

PrimitiveProcedure(error-handler-get-continue-message,0)
{
  val=continuemessage;
}

PrimitiveProcedure(error-handler-set!-jumpout,1)
{
  jumpout=ARG(0);
  val=ok_atom;
}

PrimitiveProcedure(clean-relevant-error-registers,0)
{
  errormessage=nil_constant;
  errorirritant=nil_constant;
  errorcallchain=nil_constant;
  errorenvironment=nil_constant;
  errorexpression=nil_constant;
  jumpin=nil_constant;
  continuemessage=nil_constant;
  val=ok_atom;
}

PrimitiveProcedure(open-output-string,0)
{
  val=(Pair*) createoutputstringport();
}

PrimitiveProcedure(get-output-charlist,1)
{
  if(ARG(0)->tag != OUTPUTSTRINGPORT_TYPE) {
    werr("argument error");
  } else {
    val=((OutputStringPort*) ARG(0))->charlist;
    ((OutputStringPort*) ARG(0))->charlist=nil_constant;
  }
}

PrimitiveProcedure(open-input-string,1)
{
  if(ARG(0)->tag != STRING_TYPE) {
    werr("argument-error");
  } else {
    val=(Pair*) createinputstringport(); 
    ((InputStringPort*) val)->string=make_string(C_STRINGLEN(ARG(0)),' ');
    memcpy(C_STRING(((InputStringPort*) val)->string),
	   C_STRING(ARG(0)),
	   C_STRINGLEN(ARG(0)));
  }
}

PrimitiveProcedure(sleep,1)
{
  if (ARG(0)->tag != INTEGER_TYPE) {
    werr("argument-error");
  } else {
    sleep(C_INTEGER(ARG(0)));
    val=ok_atom;
  }
}

PrimitiveProcedure(set-destructive-mode!,1)
{
  if (ARG(0) == true_constant) {
    DestructiveMacros=1;
  } else if (ARG(0) == false_constant) {
    DestructiveMacros=0;
  } else {
    werr("boolean constant expected");
  }
  val=ok_atom;
}

PrimitiveProcedure(get-destructive-mode,0)
{
  if (DestructiveMacros) {
    val=true_constant;
  } else {
    val=false_constant;
  }
}

PrimitiveProcedure(creation-date,0)
{
  val=make_string(strlen(SchemeDateCreated),' ');
  strcpy((char*)C_STRING(val),SchemeDateCreated);
}

PrimitiveProcedure(creation-time,0)
{
  val=make_string(strlen(SchemeTimeCreated),' ');
  strcpy((char*)C_STRING(val),SchemeTimeCreated);
}

PrimitiveProcedure(special-pair?,1)
{
  SchemeWord t;
  t=(ARG(0))->tag;
  if(t>=SPECIAL_START_TYPE && t<=SPECIAL_END_TYPE) {
    val=true_constant;
  } else {
    val=false_constant;
  }
}

PrimitiveProcedure(special-or-pair?,1)
{
  SchemeWord t;
  t=(ARG(0))->tag;
  if((t>=SPECIAL_START_TYPE && t<=SPECIAL_END_TYPE) || t==PAIR_TYPE) {
    val=true_constant;
  } else {
    val=false_constant;
  }
}

/*
 * Fall 1: normaler call    (#t . #<internal-continuation>)
 * Fall 2: recall 	       (#f . #<value>)
 * wird von eval.c aufgerufen ...
 */

Pair* SchemeGetInternalContinuation()
{
  Continuation* rc;
  int i,length;
  Pair **p;
  Pair *tmp;
  char *cp;

  rc=(Continuation*) AllocContinuation();
  rc->fr1=fr1;
  rc->fr2=fr2;
  rc->fr3=fr3;
  rc->fr4=fr4;
  rc->pr_a=pr_a;
  rc->pr_b=pr_b;
  rc->pr_c=pr_c;
  rc->pr_d=pr_d;
  rc->expr=expr;
  rc->env=env;
  rc->val=val;
  rc->fun=fun;
  rc->argl=argl;
  rc->newenv=newenv;
  rc->unev=unev;
  rc->initf=initf;
  rc->exitf=exitf;
  rc->expr_env=expr_env;
  rc->callchain=callchain;
 
  rc->pstack=nil_constant;
  rc->cstack=nil_constant;

  rc->parameters=nil_constant;

  length=the_pstackptr - the_pstack;  
  SchemePushP((Pair*) rc);	/* protect rc */
  tmp=make_vector(length,nil_constant);
  rc=(Continuation*) SchemePopP();
  rc->pstack=tmp;
  for(i=0, p=the_pstack; i<length; i++, p++) {
    (C_VECTOR(rc->pstack))[i]=*p;
  }

  length=the_cstackptr - the_cstack;
  SchemePushP((Pair*) rc);	/* protect rc */
  tmp=make_string(length,' ');
  rc=(Continuation*) SchemePopP();
  rc->cstack=tmp;
  for (i=0, cp=the_cstack; i<length; i++, cp++) {
    (C_STRING(rc->cstack))[i]=*cp;
  }

  /*
   * Hier wird eine Pseudo-Parameterliste definiert, die sich an der 
   * gleichen Stelle einer Compound-Procedure befindet. Dadurch wird
   * eine Continuation wie eine Procedure, die einen Paramter erwartet
   * behandelt.
   */

  {
    Pair* p;
    SchemePushP((Pair*) rc);
    SetPairIn(p);
    rc=(Continuation*) SchemePopP();
    rc->parameters=p;
    rc->parameters->car=ok_atom;
    rc->parameters->cdr=nil_constant;
  }

  return((Pair*) rc);
}

SchemeSetInternalContinuation(Continuation* continuation)
{
  int i,length;
  Pair ** p;
  char *cp;

  fr1=continuation->fr1;
  fr2=continuation->fr2;
  fr3=continuation->fr3;
  fr4=continuation->fr4;
  pr_a=continuation->pr_a;
  pr_b=continuation->pr_b;
  pr_c=continuation->pr_c;
  pr_d=continuation->pr_d;
  expr=continuation->expr;
  env=continuation->env;

  /*
   *val=continuation->val;
   */

  fun=continuation->fun;
  argl=continuation->argl;
  newenv=continuation->newenv;
  unev=continuation->unev;

  initf=continuation->initf;
  exitf=continuation->exitf;
  expr_env=continuation->expr_env;
  callchain=continuation->callchain;

  length=C_VECTORLEN(continuation->pstack);
  the_pstackptr=the_pstack + length;
  for(i=0, p=the_pstack; i<length; i++, p++) {
    *p=C_VECTOR(continuation->pstack)[i];
  }

  length=C_STRINGLEN(continuation->cstack);
  the_cstackptr=the_cstack + length;
  for (i=0, cp=the_cstack; i<length; i++, cp++) {
    *cp=C_STRING(continuation->cstack)[i];
  }
}

PrimitiveProcedure(get-current-continuation,0)
{
  SetPairIn(val);
  val->car=true_constant;
  val->cdr=nil_constant;
  val->cdr=(Pair*) SchemeGetInternalContinuation();
}

PrimitiveProcedure(continuation?,1)
{
  if (ARG(0)->tag == CONTINUATION_TYPE ||
      ARG(0)->tag == MCONTINUATION_TYPE) {
    val=true_constant;
  } else {
    val=false_constant;
  }
}

PrimitiveProcedure(get-initf,0)
{
  val=initf;
}

PrimitiveProcedure(set-initf!,1)
{
  initf=ARG(0);
  val=ok_atom;
}

PrimitiveProcedure(get-exitf,0)
{
  val=exitf;
}

PrimitiveProcedure(set-exitf!,1)
{
  exitf=ARG(0);
  val=ok_atom;
}

PrimitiveProcedure(get-pstack,1)
{
  if ((ARG(0)->tag != CONTINUATION_TYPE &&
      (ARG(0)->tag != MCONTINUATION_TYPE))) {
    werri("argument-error");
  } else {
    val=((Continuation*) (ARG(0)))->pstack;
  }
}

PrimitiveProcedure(continuation-parameters,1)
{
  if ((ARG(0)->tag != CONTINUATION_TYPE &&
      (ARG(0)->tag != MCONTINUATION_TYPE))) {
    werri("argument-error");
  } else {
    val=((Continuation*) (ARG(0)))->parameters;
  }
}

/*
 * Typedef des neuen Typen mit External am Anfang
 */

typedef struct {
  External external;
  SchemeWord length;
  char* pointer;
} Mtype;

/*
 * Definition: wie wird der Typ ausgegeben ? Ausgabe erfolgt ueber writestring.
 */

static malloc_print(Port* p, Mtype* e) {
  char buf[128];
  char adr[128];
  sprintf(adr,"0x%X",e->pointer);
  sprintf(buf,"#<malloc'ed %d bytes at adress %s>",e->length,(e->pointer?adr:"NULL"));
  writestring(p, buf);
}

/*
 * Wie wird der Typ von der GC behandelt wenn keine Referenz mehr da ist ?
 */

static malloc_destroy(Mtype* e) {
  if(e->pointer != NULL) {
    /*
    printf("freeing %X\n",e->pointer);
    */
    SchemeFree(e->pointer);
    e->pointer=NULL;
  }
}

static malloc_getchar(Mtype* e) {
  return('a');
}

/*
 * Funktionsliste identifiziert zugleich diesen Typ.
 */

static ExternalFunctions malloc_functions={
  malloc_print,
  malloc_destroy,
  NULL,			/* putchar */
  NULL,			/* putstring */
  malloc_getchar,	/* getchar */
  NULL			/* ungetchar */
};

/*
 * Beispiel einer Generierung eines externen Typs
 *
 */

PrimitiveProcedure(m-malloc,1)
{
  Mtype* e;
  char* p;
  if(ARG(0)->tag != INTEGER_TYPE) {
    werr("argument-error");
  } else {
    if((p=(char*) SchemeMalloc(C_INTEGER(ARG(0)))) == NULL ) {
      werr("cannot malloc");
    } else {
      /*
      printf("alloc'ed at %X\n",p);
      */
      /*
       * AllocExternalType erwartet Laenge der Nutzdaten in SchemeWord, also gilt folgender
       * Ausdruck:
       */
      e=(Mtype*) AllocExternalType((sizeof(Mtype) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&malloc_functions;
      e->length=(SchemeWord) C_INTEGER(ARG(0));
      e->pointer=p;
      /*
      printf("e->pointer=%X\n",e->pointer);
      */
      val=(Pair*) e;
    }  
  }
}

PrimitiveProcedure(m-free,1)
{
  Mtype* e;
  e=(Mtype*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &malloc_functions) {
    werr("argument-error");
  } else {
    if(e->pointer != NULL) { 
      SchemeFree(e->pointer);
      e->pointer=NULL;
      val=true_constant;
    } else {
      val=false_constant;
    }  
  }
}

PrimitiveProcedure(m->string,1)
{
  Mtype* e;
  e=(Mtype*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &malloc_functions) {
    werr("argument-error");
  } else {
    if(e->pointer == NULL) {
      val=false_constant;
    } else {
      val=make_string(e->length,' ');
      memcpy(C_STRING(val),e->pointer,e->length);
    }
  }
}

PrimitiveProcedure(m-length,1)
{
  Mtype* e;
  e=(Mtype*) ARG(0);
  if(ARG(0)->tag != EXTERNAL_TYPE || e->external.functions != &malloc_functions) {
    werr("argument-error");
  } else {
    if(e->pointer != NULL) {
      val=AllocInteger();
      val->car=(Pair*) e->length;
    } else {
      val=false_constant;
    }
  }
}

// - Ende vom Beispiel fuer EXTERN-TYPES ... -------------------------------------

static int lookup(char *file, char *searchstring) {
  FILE* fd;
  int c;
  int state= -1;
  int position= -1;
  int counter;

  if(strlen(searchstring) <= 1) {
    return(-1);
  }

  if((fd=fopen(file,"r")) == NULL) {
    return(-1);
  }

  for(counter=0;(c=fgetc(fd)) != EOF; counter++) {
    if(state== -1) {
      if(c == searchstring[0]) {
        state=1;
	position=counter;
	if(state==(int) strlen(searchstring)) {
	  fclose(fd);
	  return(position);
	}
      } else {
	continue;
      }
    } else {
      if(c == searchstring[state]) {
	if(state==((int) strlen(searchstring)-1)) {
	  fclose(fd);
	  return(position);
	}
	state++;
      } else {
	state= -1;
	position= -1;
	if(c == searchstring[0]) {
	  state=1;
	  position=counter;
	  if(state == (int) strlen(searchstring)) {
	    fclose(fd);
	    return(position);
	  }
	} else {
	  continue;
        }
      }
    }
  }
  fclose(fd);
  return(-1);
}

static int patch(char* filename, int position, char* string) {
  int fd;
  if((fd=open(filename, O_WRONLY, 0)) == -1) {
    return(SCHEME_FALSE);
  }
  if(lseek(fd,position,SEEK_SET) == -1) {
    close(fd);
    return(SCHEME_FALSE);
  }
  if(write(fd,string,16) == -1) {
    close(fd);
    return(SCHEME_FALSE);
  }
  close(fd);
  return(SCHEME_OK);
}

PrimitiveProcedure(exit-on-error?,0)
{
  if(exit_on_error==1) {
    val=true_constant;
  } else {
    val=false_constant;
  }
}

PrimitiveProcedure(set-exit-on-error!,1)
{
  if (ARG(0) == true_constant) {
    exit_on_error=1;
  } else if (ARG(0) == false_constant) {
    exit_on_error=0;
  } else {
    werr("boolean constant expected");
  }
  val=ok_atom;
}

//

static ES* es_create(FILE* fd) {
  ES* es;
  if((es=(ES*)SchemeMalloc(sizeof(ES))) == NULL) {
    return(NULL);
  } else {
    es->fd=fd;
    es->state=ES_STATE_START;
    es->bytecount=0;
    es->esstackptr=0;
    es->uc=-1;	
  }
  return(es); 
}

static es_push(ES* es, int c) {
  if(es->esstackptr<ES_MAXESSTACK) {
    es->esstack[es->esstackptr++]=c;
  }
}

static es_pushstring(ES* es, char* s) {
  int i;
  for (i=strlen(s)-1; i>=0; i--) {
    es_push(es,s[i]);
  }
}

static int es_pop(ES* es) {
  if(es->esstackptr == 0) {
    return(EOF);
  } else {
    return(es->esstack[--es->esstackptr]); 
  }
}

static int es_getchar(ES* es) {
  int a,b;
  int x;

  if(es->uc != -1) {
    int rc=es->uc;
    es->uc=-1;
    return(rc);
  } else {
    switch(es->state) {
      case ES_STATE_START:
        a=getc(es->fd);
	if(a==EOF) {
	  es->state=ES_STATE_EOF;
	  return(EOF);
	}
        if(a == '<') {
	  b=getc(es->fd);  
	  if(b==EOF) {
	    es->state=ES_STATE_EOF;
	    return(a);
	  }
	  if(b == '?') {
	    es->state = ES_STATE_ACTIVE;
            return(es_getchar(es));
          } else {
	    ungetc(b,es->fd);
	    goto start_cont;
	  }
	} else {
start_cont:
          // a  
	  ungetc(a,es->fd); 
          es_pushstring(es, "(display \"");
	  es->state=ES_STATE_DISPLAY;
	  return(es_pop(es));
	}
        break;

      case ES_STATE_DISPLAY:
        x=es_pop(es);
	if(x != EOF) {
	  es->bytecount++;
	  return x;
	}

        if(es->bytecount > ES_MAXINDISPLAY) {
	  es->bytecount=1;
	  es_pushstring(es,"\") (display \"");
	  return(es_pop(es));
	}

        a=getc(es->fd);
	if(a==EOF) {
	  es->state=ES_STATE_EOF;
	  es_push(es,'\051');   // Klammer zu
	  es->bytecount++;
	  return('\042');	// Doublequote
	}
        if(a == '<') {
	  b=getc(es->fd);
          if(b==EOF) {
             es->state=ES_STATE_EOF;
             // return(a);
	     goto display_charproc;
          }
          if(b == '?') {	// Klammer auf
	    es->state=ES_STATE_ACTIVE;
	    es_pushstring(es,")");
	    es->bytecount++;
	    return('\042');	// Doublequote
          } else {
	    ungetc(b,es->fd);
display_charproc:
	    if(a>=32 && a<=126 && a!=34 && a!='\\') {
	      es->bytecount++;
	      return(a);
	    } else {
	      if (a=='\\') {
		es_push(es,'\\');
		return('\\');
	      } else {
	        char buf[5];
	        sprintf(buf,"\\%03d",a);
	        es_pushstring(es,buf);
	        es->bytecount++;
	        return(es_pop(es));
	      }
	    }
          }
	} else {
	  goto display_charproc;
          // return(a);
	}
        break;

      case ES_STATE_ACTIVE:
        x=es_pop(es);
	if(x != EOF) {
	  return x;
	}
        a=getc(es->fd);
	if(a==EOF) {
	  es->state=ES_STATE_EOF;
	  return(EOF);
	}
        if(a =='!') {	
	  b=getc(es->fd);
	  if(b==EOF) {
	    es->state=ES_STATE_EOF;
	    return(a);
	  }
          if(b == '>') {
	    es->state=ES_STATE_START;
	    return(es_getchar(es));
	  } else {
	    ungetc(b,es->fd);
	    return(a);
	  }
        } else {
	  return(a);
	}
        break;

      case ES_STATE_EOF:
        x=es_pop(es);
	if(x != EOF) {
	  return x;
	}
	return(EOF);
	break;

      default:
        break;
    }
    return(EOF);
  }
}

static int es_ungetchar(ES* es, int c) {
  if(es->uc != -1) {
    fprintf(stderr,"double es_ungetchar\n");
  } else {
    es->uc=c;
  }
}

static void es_destroy(ES* es) {
  free(es);
}

// Typedef des ES-Port-Types

typedef struct {
  External external;
  ES* pointer;
} ESPorttype;

// Definition: wie wird der Typ ausgegeben ? Ausgabe erfolgt ueber writestring.
 
static esport_print(Port* p, ESPorttype* e) {
  char buf[128];
  char adr[128];
  sprintf(adr,"0x%X",e->pointer);
  sprintf(buf,"#<esport %X>",(e->pointer?adr:"NULL"));
  writestring(p, buf);
}

// Wie wird der Typ von der GC behandelt wenn keine Referenz mehr da ist ?

static esport_destroy(ESPorttype* e) {
  if(e->pointer != NULL) {
    printf("freeing %X\n",e->pointer);
    SchemeFree(e->pointer);
    e->pointer=NULL;
  }
}

static esport_getchar(ESPorttype* e) {
  // return('b');
  return(es_getchar(e->pointer));
}

// Funktionsliste identifiziert zugleich diesen Typ.

static ExternalFunctions esport_functions={
  esport_print,
  esport_destroy,
  NULL,			/* putchar */
  NULL,			/* putstring */
  esport_getchar,	/* getchar */
  NULL			/* ungetchar */
};

// Beispiel einer Generierung eines externen Typs

PrimitiveProcedure(open-embedded-input-file,1)
{
  ESPorttype* e;
  ES* es;
  FILE* fd;

  if(ARG(0)->tag != STRING_TYPE) {
    werr("argument-error");
  } else {
    if((fd=fopen((char*) C_STRING(ARG(0)),"r")) == NULL) {
      werr("cannot open");
      return;
    }
    if((es=es_create(fd)) == NULL ) {
      werr("cannot create esport");
      return;
    } else {

      // AllocExternalType erwartet Laenge der Nutzdaten in SchemeWord...

      e=(ESPorttype*) AllocExternalType((sizeof(ESPorttype) - sizeof(External))/sizeof(SchemeWord));
      e->external.functions=&esport_functions;
      e->pointer=es;

      printf("esport created %X\n",e->pointer);

      val=(Pair*) e;
    }  
  }
}


PrimitiveProcedure(escheme?,0)
{
  int l=strlen(global_argv[0]);
  if(l >= 7) {
    if(!strncmp(global_argv[0]+l-7,"escheme",7)) {
      val=true_constant;
    } else {
      val=false_constant;
    }
  } else {
    val=false_constant;
  }
}



